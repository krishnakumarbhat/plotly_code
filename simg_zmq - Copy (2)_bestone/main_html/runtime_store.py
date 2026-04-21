import json
import os
import sqlite3
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, Optional

try:
    from .env_utils import get_db_path
except ImportError:
    from env_utils import get_db_path


class RuntimeStore:
    def __init__(self, db_path: Optional[str] = None):
        self.db_path = db_path or os.environ.get('HPCC_RUNTIME_DB') or get_db_path()
        self.repo_root = Path(__file__).resolve().parents[1]
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
        runtime_dir = self.repo_root / 'simg_sh_hpcc'
        defaults = [
            {
                'tool_key': 'main_html',
                'display_name': 'Main HTML Service',
                'category': 'service',
                'image_path': str(runtime_dir / 'main_html.simg'),
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
                'image_path': str(runtime_dir / 'can_kpi.simg'),
                'entry_command': 'singularity run can_kpi.simg <mode> ...',
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
                'image_path': str(runtime_dir / 'udp_kpi.simg'),
                'entry_command': 'singularity run udp_kpi.simg <mode> ...',
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
                'image_path': str(runtime_dir / 'interactiveplot.simg'),
                'entry_command': 'singularity run interactiveplot.simg <xml> <json> <out>',
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
                'image_path': str(runtime_dir / 'rag.simg'),
                'entry_command': 'singularity run rag.simg --talk',
                'service_url': 'http://127.0.0.1:5100',
                'input_hint': 'HTML root for ingestion + user question',
                'output_hint': 'Answer JSON + Chroma/SQLite data',
                'notes': 'Chat requests from main_html are forwarded here',
                'metadata_json': {'port': 5100, 'default_args': '--talk', 'node_color': '#dfe7f4'},
            },
        ]

        for item in defaults:
            existing = self.get_tool(item['tool_key'])
            if not existing:
                self.save_tool(item, updated_by='system')

        self._ensure_default_graph_variant()

    def list_tools(self) -> List[Dict[str, Any]]:
        with self._connect() as connection:
            rows = connection.execute(
                "SELECT * FROM runtime_tools ORDER BY CASE category WHEN 'service' THEN 0 WHEN 'batch' THEN 1 ELSE 2 END, display_name"
            ).fetchall()
        return [self._row_to_tool(row) for row in rows]

    def get_tool(self, tool_key: str) -> Optional[Dict[str, Any]]:
        with self._connect() as connection:
            row = connection.execute(
                'SELECT * FROM runtime_tools WHERE tool_key = ?',
                (tool_key,),
            ).fetchone()
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

    def list_graph_variants(self) -> List[Dict[str, Any]]:
        with self._connect() as connection:
            rows = connection.execute(
                'SELECT * FROM runtime_graph_variants ORDER BY is_default DESC, display_name'
            ).fetchall()
        return [self._row_to_variant(row) for row in rows]

    def get_graph_variant(self, variant_key: Optional[str] = None) -> Optional[Dict[str, Any]]:
        with self._connect() as connection:
            if variant_key:
                row = connection.execute(
                    'SELECT * FROM runtime_graph_variants WHERE variant_key = ?',
                    (variant_key,),
                ).fetchone()
            else:
                row = connection.execute(
                    'SELECT * FROM runtime_graph_variants ORDER BY is_default DESC, display_name LIMIT 1'
                ).fetchone()
        return self._row_to_variant(row) if row else None

    def save_graph_variant(self, payload: Dict[str, Any], updated_by: str = 'system') -> Dict[str, Any]:
        variant_key = str(payload['variant_key']).strip()
        timestamp = self._utcnow()
        variants = self.list_graph_variants()
        is_default = bool(payload.get('is_default')) or not variants
        graph_state = self._merge_graph_state(payload.get('graph', {}), self.list_tools())

        with self._connect() as connection:
            if is_default:
                connection.execute('UPDATE runtime_graph_variants SET is_default = 0')

            connection.execute(
                """
                INSERT INTO runtime_graph_variants (
                    variant_key, display_name, description, graph_json,
                    is_default, updated_by, updated_at
                ) VALUES (?, ?, ?, ?, ?, ?, ?)
                ON CONFLICT(variant_key) DO UPDATE SET
                    display_name = excluded.display_name,
                    description = excluded.description,
                    graph_json = excluded.graph_json,
                    is_default = excluded.is_default,
                    updated_by = excluded.updated_by,
                    updated_at = excluded.updated_at
                """,
                (
                    variant_key,
                    payload.get('display_name', variant_key),
                    payload.get('description', ''),
                    json.dumps(graph_state),
                    1 if is_default else 0,
                    updated_by,
                    timestamp,
                ),
            )

        return self.get_graph_variant(variant_key)

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
            connection.execute(
                f"UPDATE runtime_jobs SET {', '.join(columns)} WHERE id = ?",
                tuple(values),
            )
        return self.get_job(runtime_job_id)

    def append_event(self, runtime_job_id: int, level: str, message: str) -> None:
        with self._connect() as connection:
            connection.execute(
                'INSERT INTO runtime_events (runtime_job_id, level, message, created_at) VALUES (?, ?, ?, ?)',
                (runtime_job_id, level, message, self._utcnow()),
            )

    def get_job(self, runtime_job_id: int) -> Optional[Dict[str, Any]]:
        with self._connect() as connection:
            row = connection.execute(
                'SELECT * FROM runtime_jobs WHERE id = ?',
                (runtime_job_id,),
            ).fetchone()
        return self._row_to_job(row) if row else None

    def list_jobs(self, limit: int = 50) -> List[Dict[str, Any]]:
        with self._connect() as connection:
            rows = connection.execute(
                'SELECT * FROM runtime_jobs ORDER BY id DESC LIMIT ?',
                (limit,),
            ).fetchall()
        return [self._row_to_job(row) for row in rows]

    def get_events(self, runtime_job_id: int) -> List[Dict[str, Any]]:
        with self._connect() as connection:
            rows = connection.execute(
                'SELECT level, message, created_at FROM runtime_events WHERE runtime_job_id = ? ORDER BY id',
                (runtime_job_id,),
            ).fetchall()
        return [dict(row) for row in rows]

    def graph_payload(self, variant_key: Optional[str] = None) -> Dict[str, Any]:
        self._ensure_default_graph_variant()
        tools = self.list_tools()
        jobs = self.list_jobs(limit=20)
        variants = self.list_graph_variants()
        variant = self.get_graph_variant(variant_key) or self.get_graph_variant()
        merged_graph = self._merge_graph_state((variant or {}).get('graph', {}), tools)

        return {
            'tools': tools,
            'jobs': jobs,
            'variants': variants,
            'active_variant': (variant or {}).get('variant_key', 'default'),
            'selected_variant': variant or {},
            'nodes': merged_graph['nodes'],
            'edges': merged_graph['edges'],
            'viewport': merged_graph['viewport'],
            'graph': merged_graph,
        }

    def _ensure_default_graph_variant(self) -> None:
        if self.get_graph_variant() is not None:
            return
        tools = self.list_tools()
        self.save_graph_variant(
            {
                'variant_key': 'default',
                'display_name': 'Default topology',
                'description': 'Built-in HPCC runtime topology',
                'graph': self._default_graph_state(tools),
                'is_default': True,
            },
            updated_by='system',
        )

    def _default_graph_state(self, tools: List[Dict[str, Any]]) -> Dict[str, Any]:
        return self._merge_graph_state(
            {
                'nodes': [
                    {'id': tool['tool_key'], 'position': self._default_position(tool['tool_key'], index)}
                    for index, tool in enumerate(tools)
                ],
                'edges': [
                    {'id': 'user-main', 'source': 'main_html', 'target': 'can_kpi', 'label': 'KPI request', 'notes': ''},
                    {'id': 'user-udp', 'source': 'main_html', 'target': 'udp_kpi', 'label': 'UDP KPI request', 'notes': ''},
                    {'id': 'user-int', 'source': 'main_html', 'target': 'interactive_plot', 'label': 'Interactive plot', 'notes': ''},
                    {'id': 'user-rag', 'source': 'main_html', 'target': 'rag', 'label': 'Chat / RAG', 'notes': ''},
                ],
                'viewport': {'x': 0, 'y': 0, 'zoom': 1},
            },
            tools,
        )

    def _merge_graph_state(self, graph_state: Dict[str, Any], tools: List[Dict[str, Any]]) -> Dict[str, Any]:
        raw_nodes = list((graph_state or {}).get('nodes') or [])
        raw_edges = list((graph_state or {}).get('edges') or [])
        viewport = dict((graph_state or {}).get('viewport') or {'x': 0, 'y': 0, 'zoom': 1})
        tool_keys = {tool['tool_key'] for tool in tools}

        merged_nodes: List[Dict[str, Any]] = []
        seen_nodes = set()
        for index, node in enumerate(raw_nodes):
            node_id = str(node.get('id', '')).strip()
            if not node_id or node_id in seen_nodes or node_id not in tool_keys:
                continue
            fallback_position = self._default_position(node_id, index)
            position = dict(node.get('position') or fallback_position)
            merged_nodes.append(
                {
                    'id': node_id,
                    'position': {
                        'x': float(position.get('x', fallback_position['x'])),
                        'y': float(position.get('y', fallback_position['y'])),
                    },
                }
            )
            seen_nodes.add(node_id)

        for index, tool in enumerate(tools):
            if tool['tool_key'] in seen_nodes:
                continue
            merged_nodes.append(
                {
                    'id': tool['tool_key'],
                    'position': self._default_position(tool['tool_key'], len(merged_nodes) + index),
                }
            )

        merged_edges: List[Dict[str, Any]] = []
        seen_edges = set()
        for edge in raw_edges:
            edge_id = str(edge.get('id', '')).strip() or f"edge-{edge.get('source', '')}-{edge.get('target', '')}"
            source = str(edge.get('source', '')).strip()
            target = str(edge.get('target', '')).strip()
            if not source or not target or source not in tool_keys or target not in tool_keys:
                continue
            if edge_id in seen_edges:
                continue
            merged_edges.append(
                {
                    'id': edge_id,
                    'source': source,
                    'target': target,
                    'label': str(edge.get('label', '')).strip() or f'{source} -> {target}',
                    'notes': str(edge.get('notes', '')).strip(),
                }
            )
            seen_edges.add(edge_id)

        return {
            'nodes': merged_nodes,
            'edges': merged_edges,
            'viewport': {
                'x': float(viewport.get('x', 0)),
                'y': float(viewport.get('y', 0)),
                'zoom': float(viewport.get('zoom', 1)),
            },
        }

    def _default_position(self, tool_key: str, index: int) -> Dict[str, float]:
        layout = {
            'main_html': {'x': 280.0, 'y': 40.0},
            'can_kpi': {'x': 40.0, 'y': 240.0},
            'udp_kpi': {'x': 280.0, 'y': 240.0},
            'interactive_plot': {'x': 520.0, 'y': 240.0},
            'rag': {'x': 520.0, 'y': 40.0},
            'hyperlink': {'x': 40.0, 'y': 40.0},
        }
        if tool_key in layout:
            return dict(layout[tool_key])

        row = index // 4
        col = index % 4
        return {'x': float(40 + (col * 240)), 'y': float(420 + (row * 180))}

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

    def _row_to_variant(self, row: sqlite3.Row) -> Dict[str, Any]:
        return {
            'variant_key': row['variant_key'],
            'display_name': row['display_name'],
            'description': row['description'] or '',
            'graph': self._loads(row['graph_json'], {'nodes': [], 'edges': [], 'viewport': {'x': 0, 'y': 0, 'zoom': 1}}),
            'is_default': bool(row['is_default']),
            'updated_by': row['updated_by'],
            'updated_at': row['updated_at'],
        }

    def _row_to_job(self, row: sqlite3.Row) -> Dict[str, Any]:
        return {
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
            'events': self.get_events(int(row['id'])),
        }