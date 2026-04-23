import json
import os
import re
import sqlite3
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, Optional


def _invocation_root() -> Path:
    argv0 = Path(sys.argv[0]).resolve() if sys.argv and sys.argv[0] else Path(__file__).resolve()
    if argv0.is_file():
        return argv0.parent
    return argv0 if argv0.is_dir() else argv0.parent


def _project_root() -> Path:
    override = (os.environ.get('HPCC_PROJECT_ROOT') or '').strip()
    if override:
        return Path(override).resolve()

    invocation_root = _invocation_root()
    bundle_src = invocation_root / 'bundle_src'
    if bundle_src.is_dir():
        return bundle_src.resolve()

    for candidate in (invocation_root, Path(__file__).resolve().parent, Path(__file__).resolve().parent.parent):
        if (candidate / 'main_html').is_dir() or (candidate / 'KPI').is_dir() or (candidate / 'rag').is_dir():
            return candidate.resolve()

    return invocation_root.resolve()


def _runtime_dir(repo_root: Path) -> Path:
    bundle_override = (os.environ.get('HPCC_BUNDLE_ROOT') or '').strip()
    if bundle_override:
        return Path(bundle_override).resolve()

    invocation_root = _invocation_root()
    parent_bundle_root = repo_root.parent if repo_root.name == 'bundle_src' else None
    for candidate in (invocation_root, repo_root, parent_bundle_root):
        if candidate and ((candidate / 'main_html.simg').exists() or (candidate / 'hpcc_main.pyz').exists()):
            return candidate.resolve()

    runtime_dir = repo_root / 'simg_sh_hpcc'
    if runtime_dir.is_dir():
        return runtime_dir.resolve()
    return runtime_dir


def _default_db_path() -> str:
    runtime_dir = _runtime_dir(_project_root())
    cache_dir = runtime_dir / 'runtime_state' / 'main_html' / 'cache_html'
    cache_dir.mkdir(parents=True, exist_ok=True)
    return str(cache_dir / 'hpc_tools_dev.db')


class RuntimeStore:
    ACTIVE_JOB_STATUSES = {'QUEUED', 'SUBMITTED', 'PENDING', 'RUNNING'}

    def __init__(self, db_path: Optional[str] = None):
        self.repo_root = _project_root()
        self.runtime_dir = _runtime_dir(self.repo_root)
        self.db_path = db_path or os.environ.get('HPCC_RUNTIME_DB') or _default_db_path()
        Path(self.db_path).parent.mkdir(parents=True, exist_ok=True)
        self._ensure_schema()

    def _connect(self) -> sqlite3.Connection:
        connection = sqlite3.connect(self.db_path)
        connection.row_factory = sqlite3.Row
        return connection

    def _ensure_schema(self) -> None:
        with self._connect() as connection:
            connection.executescript(
                """
                CREATE TABLE IF NOT EXISTS runtime_tools (
                    tool_key TEXT PRIMARY KEY,
                    display_name TEXT NOT NULL,
                    category TEXT NOT NULL,
                    image_path TEXT,
                    entry_command TEXT,
                    service_url TEXT,
                    input_hint TEXT,
                    output_hint TEXT,
                    notes TEXT,
                    metadata_json TEXT,
                    updated_by TEXT,
                    updated_at TEXT NOT NULL
                );

                CREATE TABLE IF NOT EXISTS runtime_jobs (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    tool_key TEXT NOT NULL,
                    requested_by TEXT NOT NULL,
                    session_id TEXT,
                    status TEXT NOT NULL,
                    mode TEXT,
                    input_path TEXT,
                    output_path TEXT,
                    log_path TEXT,
                    command_json TEXT,
                    resources_json TEXT,
                    request_json TEXT,
                    pid INTEGER,
                    return_code INTEGER,
                    error_message TEXT,
                    created_at TEXT NOT NULL,
                    started_at TEXT,
                    completed_at TEXT
                );

                CREATE TABLE IF NOT EXISTS runtime_events (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    runtime_job_id INTEGER NOT NULL,
                    level TEXT NOT NULL,
                    message TEXT NOT NULL,
                    created_at TEXT NOT NULL,
                    FOREIGN KEY(runtime_job_id) REFERENCES runtime_jobs(id)
                );

                CREATE TABLE IF NOT EXISTS runtime_graph_variants (
                    variant_key TEXT PRIMARY KEY,
                    display_name TEXT NOT NULL,
                    description TEXT,
                    graph_json TEXT NOT NULL,
                    is_default INTEGER NOT NULL DEFAULT 0,
                    updated_by TEXT,
                    updated_at TEXT NOT NULL
                );
                """
            )

    def ensure_defaults(self) -> None:
        defaults = [
            {
                'tool_key': 'main_html',
                'display_name': 'Main HTML Service',
                'category': 'service',
                'image_path': str(self.runtime_dir / 'main_html.simg'),
                'entry_command': 'singularity run main_html.simg',
                'service_url': 'http://127.0.0.1:5001/html',
                'input_hint': 'Browser traffic only',
                'output_hint': 'Flask UI on port 5001',
                'notes': 'Main login + dashboard service',
                'metadata_json': {'port': 5001, 'node_color': '#d4efe8'},
            },
            {
                'tool_key': 'can_kpi',
                'display_name': 'CAN KPI',
                'category': 'batch',
                'image_path': str(self.runtime_dir / 'kpi' / 'can' / 'can_kpi.simg'),
                'entry_command': 'singularity run kpi/can/can_kpi.simg <mode> ...',
                'service_url': '',
                'input_hint': 'JSON batch or input/output HDF pair',
                'output_hint': 'HTML KPI report directory',
                'notes': 'Uses KPI/can_kpi/kpi_main.py',
                'metadata_json': {'default_mode': 'json', 'default_args': '', 'node_color': '#f3efe4'},
            },
            {
                'tool_key': 'udp_kpi',
                'display_name': 'UDP KPI',
                'category': 'batch',
                'image_path': str(self.runtime_dir / 'kpi' / 'udp' / 'udp_kpi.simg'),
                'entry_command': 'singularity run kpi/udp/udp_kpi.simg <mode> ...',
                'service_url': '',
                'input_hint': 'JSON batch, HDF pair, or ZMQ bridge mode',
                'output_hint': 'HTML KPI report directory',
                'notes': 'Uses KPI/UDP_KPI/kpi_server.py',
                'metadata_json': {'default_mode': 'json', 'default_args': '', 'node_color': '#e7f3ef'},
            },
            {
                'tool_key': 'interactive_plot',
                'display_name': 'Interactive Plot',
                'category': 'batch',
                'image_path': str(self.runtime_dir / 'kpi' / 'int_plot' / 'intplot_kpi.simg'),
                'entry_command': 'singularity run kpi/int_plot/intplot_kpi.simg <xml> <json> <out>',
                'service_url': '',
                'input_hint': 'Config XML + inputs JSON, or HDF pair',
                'output_hint': 'Interactive HTML plot directory',
                'notes': 'Can bridge to UDP KPI over ZMQ',
                'metadata_json': {'default_mode': 'json', 'default_args': '', 'node_color': '#eef5ee'},
            },
            {
                'tool_key': 'hyperlink',
                'display_name': 'Hyperlink Viewer',
                'category': 'viewer',
                'image_path': '',
                'entry_command': 'served inside main_html',
                'service_url': '/hyperlink/',
                'input_hint': 'HTML directory with optional video directory',
                'output_hint': 'Viewer session + copied html/video cache',
                'notes': 'Integrated directly into main_html routes',
                'metadata_json': {'default_args': '', 'node_color': '#f6ead8'},
            },
            {
                'tool_key': 'rag',
                'display_name': 'RAG Service',
                'category': 'service',
                'image_path': str(self.runtime_dir / 'rag' / 'rag.simg'),
                'entry_command': 'singularity run rag/rag.simg --talk',
                'service_url': '',
                'input_hint': 'HTML root for ingestion + user question',
                'output_hint': 'Answer JSON + Chroma/SQLite data',
                'notes': 'GPU-backed service launched on demand through the HPCC broker',
                'metadata_json': {'port': 5100, 'default_args': '--talk', 'gpu': True, 'node_color': '#dfe7f4'},
            },
        ]

        for item in defaults:
            existing = self.get_tool(item['tool_key'])
            should_refresh = (
                existing
                and (existing.get('updated_by') or 'system') == 'system'
                and (
                    existing.get('image_path') != item.get('image_path')
                    or existing.get('entry_command') != item.get('entry_command')
                    or existing.get('service_url') != item.get('service_url')
                )
            )
            if not existing or should_refresh:
                self.save_tool(item, updated_by='system')

    def list_tools(self) -> List[Dict[str, Any]]:
        with self._connect() as connection:
            rows = connection.execute(
                "SELECT * FROM runtime_tools ORDER BY CASE category WHEN 'service' THEN 0 WHEN 'batch' THEN 1 ELSE 2 END, display_name"
            ).fetchall()
        return [self._row_to_tool(row) for row in rows]

    def get_tool(self, tool_key: str) -> Optional[Dict[str, Any]]:
        with self._connect() as connection:
            row = connection.execute('SELECT * FROM runtime_tools WHERE tool_key = ?', (tool_key,)).fetchone()
        return self._row_to_tool(row) if row else None

    def save_tool(self, payload: Dict[str, Any], updated_by: str = 'system') -> Dict[str, Any]:
        timestamp = self._utcnow()
        metadata = payload.get('metadata_json', payload.get('metadata', {})) or {}
        with self._connect() as connection:
            connection.execute(
                """
                INSERT INTO runtime_tools (
                    tool_key, display_name, category, image_path, entry_command,
                    service_url, input_hint, output_hint, notes, metadata_json,
                    updated_by, updated_at
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                ON CONFLICT(tool_key) DO UPDATE SET
                    display_name = excluded.display_name,
                    category = excluded.category,
                    image_path = excluded.image_path,
                    entry_command = excluded.entry_command,
                    service_url = excluded.service_url,
                    input_hint = excluded.input_hint,
                    output_hint = excluded.output_hint,
                    notes = excluded.notes,
                    metadata_json = excluded.metadata_json,
                    updated_by = excluded.updated_by,
                    updated_at = excluded.updated_at
                """,
                (
                    payload['tool_key'],
                    payload['display_name'],
                    payload['category'],
                    payload.get('image_path', ''),
                    payload.get('entry_command', ''),
                    payload.get('service_url', ''),
                    payload.get('input_hint', ''),
                    payload.get('output_hint', ''),
                    payload.get('notes', ''),
                    json.dumps(metadata),
                    updated_by,
                    timestamp,
                ),
            )
        return self.get_tool(payload['tool_key'])

    def create_job(
        self,
        tool_key: str,
        requested_by: str,
        session_id: str,
        mode: str,
        input_path: str,
        output_path: str,
        log_path: str,
        command: List[str],
        resources: Dict[str, Any],
        request_payload: Dict[str, Any],
        status: str = 'QUEUED',
    ) -> int:
        with self._connect() as connection:
            cursor = connection.execute(
                """
                INSERT INTO runtime_jobs (
                    tool_key, requested_by, session_id, status, mode, input_path,
                    output_path, log_path, command_json, resources_json,
                    request_json, created_at
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                """,
                (
                    tool_key,
                    requested_by,
                    session_id,
                    status,
                    mode,
                    input_path,
                    output_path,
                    log_path,
                    json.dumps(command),
                    json.dumps(resources),
                    json.dumps(request_payload),
                    self._utcnow(),
                ),
            )
            return int(cursor.lastrowid)

    def update_job(self, runtime_job_id: int, **fields: Any) -> Optional[Dict[str, Any]]:
        if not fields:
            return self.get_job(runtime_job_id)
        columns = []
        values = []
        for key, value in fields.items():
            if key.endswith('_json') and not isinstance(value, str):
                value = json.dumps(value)
            columns.append(f'{key} = ?')
            values.append(value)
        values.append(runtime_job_id)
        with self._connect() as connection:
            connection.execute(f"UPDATE runtime_jobs SET {', '.join(columns)} WHERE id = ?", tuple(values))
        return self.get_job(runtime_job_id)

    def append_event(self, runtime_job_id: int, level: str, message: str) -> None:
        with self._connect() as connection:
            connection.execute(
                'INSERT INTO runtime_events (runtime_job_id, level, message, created_at) VALUES (?, ?, ?, ?)',
                (runtime_job_id, level, message, self._utcnow()),
            )

    def get_job(self, runtime_job_id: int) -> Optional[Dict[str, Any]]:
        with self._connect() as connection:
            row = connection.execute('SELECT * FROM runtime_jobs WHERE id = ?', (runtime_job_id,)).fetchone()
        return self._row_to_job(row) if row else None

    def list_jobs(self, limit: int = 50) -> List[Dict[str, Any]]:
        with self._connect() as connection:
            rows = connection.execute('SELECT * FROM runtime_jobs ORDER BY id DESC LIMIT ?', (limit,)).fetchall()
        return [self._row_to_job(row) for row in rows]

    @staticmethod
    def _utcnow() -> str:
        return datetime.now(timezone.utc).isoformat().replace('+00:00', 'Z')

    @staticmethod
    def _loads(value: Optional[str], default: Any) -> Any:
        if not value:
            return default
        try:
            return json.loads(value)
        except (TypeError, json.JSONDecodeError):
            return default

    def _row_to_tool(self, row: sqlite3.Row) -> Dict[str, Any]:
        metadata = self._loads(row['metadata_json'], {})
        if not isinstance(metadata, dict):
            metadata = {}
        return {
            'tool_key': row['tool_key'],
            'display_name': row['display_name'],
            'category': row['category'],
            'image_path': row['image_path'],
            'entry_command': row['entry_command'],
            'service_url': row['service_url'],
            'input_hint': row['input_hint'],
            'output_hint': row['output_hint'],
            'notes': row['notes'],
            'metadata': metadata,
            'updated_by': row['updated_by'],
            'updated_at': row['updated_at'],
        }

    def _row_to_job(self, row: sqlite3.Row) -> Dict[str, Any]:
        job = {
            'id': row['id'],
            'tool_key': row['tool_key'],
            'requested_by': row['requested_by'],
            'session_id': row['session_id'],
            'status': row['status'],
            'mode': row['mode'],
            'input_path': row['input_path'],
            'output_path': row['output_path'],
            'log_path': row['log_path'],
            'command': self._loads(row['command_json'], []),
            'resources': self._loads(row['resources_json'], {}),
            'request': self._loads(row['request_json'], {}),
            'pid': row['pid'],
            'return_code': row['return_code'],
            'error_message': row['error_message'],
            'created_at': row['created_at'],
            'started_at': row['started_at'],
            'completed_at': row['completed_at'],
        }
        return self._derive_job_state(job)

    def _derive_job_state(self, job: Dict[str, Any]) -> Dict[str, Any]:
        if job.get('status') not in self.ACTIVE_JOB_STATUSES:
            return job

        log_path_value = str(job.get('log_path') or '').strip()
        if not log_path_value:
            return job

        log_path = Path(log_path_value)
        run_dir = log_path.parent
        launcher_log_path = run_dir / 'launcher.log'
        is_slurm_job = (
            str((job.get('resources') or {}).get('scheduler') or '').strip().lower() == 'slurm'
            or (run_dir / 'slurm_tmux_launcher.sh').exists()
            or (launcher_log_path.exists() and launcher_log_path != log_path)
        )
        if not is_slurm_job:
            return job

        console_head = self._read_text_window(log_path, max_bytes=16384, from_start=True)
        console_tail = self._read_text_window(log_path, max_bytes=32768, from_start=False)
        launcher_head = (
            self._read_text_window(launcher_log_path, max_bytes=16384, from_start=True)
            if launcher_log_path.exists() and launcher_log_path != log_path
            else console_head
        )
        launcher_tail = (
            self._read_text_window(launcher_log_path, max_bytes=32768, from_start=False)
            if launcher_log_path.exists() and launcher_log_path != log_path
            else console_tail
        )

        console_text = console_head + '\n' + console_tail
        start_time = self._extract_timestamp(
            console_text,
            r'^(?:\[(?:MAIN|UDP|INTERACTIVE)\]\s+)?\[(?:MAIN|UDP|INTERACTIVE)\] START: (.+)$',
        )
        has_runtime_output = self._has_runtime_output(console_text)
        if start_time and not job.get('started_at'):
            job['started_at'] = start_time
        elif has_runtime_output and not job.get('started_at'):
            job['started_at'] = self._path_mtime(log_path) or self._path_mtime(launcher_log_path)

        exit_status, exit_path = self._read_exit_status(run_dir)
        if exit_status is not None:
            job['status'] = 'COMPLETED' if exit_status == 0 else 'FAILED'
            if job.get('return_code') is None:
                job['return_code'] = exit_status
            if not job.get('completed_at'):
                job['completed_at'] = (
                    self._extract_timestamp(launcher_tail, r'^\[broker\] END: (.+)$')
                    or self._path_mtime(exit_path)
                    or self._path_mtime(log_path)
                )
            job.pop('status_detail', None)
            return job

        broker_return_code = self._extract_return_code(launcher_tail)
        if broker_return_code is not None:
            job['status'] = 'COMPLETED' if broker_return_code == 0 else 'FAILED'
            if job.get('return_code') is None:
                job['return_code'] = broker_return_code
            if not job.get('completed_at'):
                job['completed_at'] = (
                    self._extract_timestamp(launcher_tail, r'^\[broker\] END: (.+)$')
                    or self._path_mtime(launcher_log_path)
                    or self._path_mtime(log_path)
                )
            job.pop('status_detail', None)
            return job

        launcher_failure_code, launcher_failure_detail = self._extract_launcher_failure(launcher_head + '\n' + launcher_tail)
        if launcher_failure_code is not None:
            job['status'] = 'FAILED'
            if job.get('return_code') is None:
                job['return_code'] = launcher_failure_code
            if launcher_failure_detail and not job.get('error_message'):
                job['error_message'] = launcher_failure_detail
            if not job.get('completed_at'):
                job['completed_at'] = (
                    self._extract_timestamp(launcher_tail, r'^\[broker\] END: (.+)$')
                    or self._path_mtime(launcher_log_path)
                    or self._path_mtime(log_path)
                )
            job.pop('status_detail', None)
            return job

        if start_time or has_runtime_output:
            job['status'] = 'RUNNING'
            job.pop('status_detail', None)
            return job

        job['status'] = 'PENDING'
        job['status_detail'] = self._extract_wait_detail(launcher_head + '\n' + launcher_tail)
        return job

    @staticmethod
    def _read_text_window(path: Path, max_bytes: int, from_start: bool) -> str:
        if max_bytes <= 0 or not path.exists() or not path.is_file():
            return ''

        try:
            with path.open('rb') as handle:
                if from_start:
                    data = handle.read(max_bytes)
                else:
                    size = handle.seek(0, os.SEEK_END)
                    handle.seek(max(size - max_bytes, 0))
                    data = handle.read(max_bytes)
        except OSError:
            return ''

        return data.decode('utf-8', errors='replace')

    @staticmethod
    def _normalize_timestamp(raw_value: str) -> str:
        candidate = (raw_value or '').strip()
        if not candidate:
            return ''

        parsed = RuntimeStore._parse_datetime(candidate)
        if parsed is None:
            return ''

        if parsed.tzinfo is None:
            parsed = parsed.replace(tzinfo=timezone.utc)

        return parsed.astimezone(timezone.utc).isoformat().replace('+00:00', 'Z')

    @staticmethod
    def _parse_datetime(raw_value: str) -> Optional[datetime]:
        candidate = (raw_value or '').strip()
        if not candidate:
            return None

        normalized = candidate.replace('Z', '+00:00')
        parser = getattr(datetime, 'fromisoformat', None)
        if parser is not None:
            try:
                return parser(normalized)
            except ValueError:
                pass

        if len(normalized) >= 6 and normalized[-6] in '+-' and normalized[-3] == ':':
            normalized = normalized[:-3] + normalized[-2:]

        formats = (
            '%Y-%m-%dT%H:%M:%S.%f%z',
            '%Y-%m-%dT%H:%M:%S%z',
            '%Y-%m-%d %H:%M:%S.%f%z',
            '%Y-%m-%d %H:%M:%S%z',
            '%Y-%m-%dT%H:%M:%S.%f',
            '%Y-%m-%dT%H:%M:%S',
            '%Y-%m-%d %H:%M:%S.%f',
            '%Y-%m-%d %H:%M:%S',
        )
        for fmt in formats:
            try:
                return datetime.strptime(normalized, fmt)
            except ValueError:
                continue
        return None

    def _extract_timestamp(self, text: str, pattern: str) -> str:
        match = re.search(pattern, text or '', re.MULTILINE)
        if not match:
            return ''
        return self._normalize_timestamp(match.group(1))

    @staticmethod
    def _has_runtime_output(text: str) -> bool:
        for line in (text or '').splitlines():
            normalized = line.strip()
            if not normalized:
                continue
            if re.match(
                r'^(?:\[(?:MAIN|UDP|INTERACTIVE)\]\s+)?\[(?:MAIN|UDP|INTERACTIVE)\] (COMMAND|START|EXIT_STATUS):',
                normalized,
            ):
                continue
            if normalized.startswith(('[MAIN]', '[UDP]', '[INTERACTIVE]')):
                return True
        return False

    @staticmethod
    def _extract_return_code(text: str) -> Optional[int]:
        match = re.search(r'^\[broker\] RETURN_CODE: (-?\d+)$', text or '', re.MULTILINE)
        if not match:
            return None
        try:
            return int(match.group(1))
        except ValueError:
            return None

    @staticmethod
    def _extract_launcher_failure(text: str):
        detail = ''
        for line in reversed((text or '').splitlines()):
            normalized = line.strip()
            lowered = normalized.lower()
            if not normalized:
                continue
            if normalized.startswith('srun: error:') or 'error connecting to /tmp/tmux-' in lowered:
                detail = normalized
                break

        match = re.search(r'exited with exit code (\d+)', text or '', re.IGNORECASE)
        if match:
            try:
                return int(match.group(1)), detail or match.group(0)
            except ValueError:
                return None, detail

        if detail:
            return 1, detail

        return None, ''

    @staticmethod
    def _path_mtime(path: Optional[Path]) -> str:
        if path is None:
            return ''
        try:
            modified_at = datetime.fromtimestamp(path.stat().st_mtime, tz=timezone.utc)
        except OSError:
            return ''
        return modified_at.isoformat().replace('+00:00', 'Z')

    @staticmethod
    def _extract_wait_detail(text: str) -> str:
        for line in (text or '').splitlines():
            normalized = line.strip()
            lowered = normalized.lower()
            if not normalized:
                continue
            if 'queued and waiting for resources' in lowered:
                return 'Waiting for Slurm resources'
            if normalized.startswith('srun: job '):
                return normalized
        return 'Waiting for Slurm allocation'

    @staticmethod
    def _read_exit_status(run_dir: Path):
        exit_codes: Dict[str, int] = {}
        exit_paths: Dict[str, Path] = {}
        for name in ('compute_main.exit', 'compute_interactive.exit', 'compute_udp.exit'):
            path = run_dir / name
            if not path.exists():
                continue
            try:
                raw_value = path.read_text(encoding='utf-8', errors='replace').strip()
                exit_codes[name] = int(raw_value)
                exit_paths[name] = path
            except (OSError, ValueError):
                continue

        if not exit_codes:
            return None, None

        for name in ('compute_main.exit', 'compute_interactive.exit', 'compute_udp.exit'):
            code = exit_codes.get(name)
            if code is not None and code != 0:
                return code, exit_paths.get(name)

        for name in ('compute_interactive.exit', 'compute_main.exit', 'compute_udp.exit'):
            code = exit_codes.get(name)
            if code is not None:
                return code, exit_paths.get(name)

        return None, None