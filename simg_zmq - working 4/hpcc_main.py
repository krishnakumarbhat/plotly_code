import argparse
import json
import os
import shlex
import shutil
import signal
import socket
import socketserver
import subprocess
import sys
import threading
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

from main_html.runtime_store import RuntimeStore


def _to_wsl_path(path_value: Path) -> str:
    resolved = str(path_value.resolve())
    normalized = resolved.replace('\\', '/')
    if len(normalized) >= 2 and normalized[1] == ':':
        drive = normalized[0].lower()
        suffix = normalized[2:]
        if suffix.startswith('/'):
            suffix = suffix[1:]
        return f'/mnt/{drive}/{suffix}'
    return normalized


def _runtime_binary_name() -> str:
    if shutil.which('apptainer'):
        return 'apptainer'
    if shutil.which('singularity'):
        return 'singularity'
    return 'apptainer'


def _wsl_available() -> bool:
    return os.name == 'nt' and shutil.which('wsl') is not None


def _local_ipv4_address() -> Optional[str]:
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as connection:
            connection.connect(('8.8.8.8', 80))
            candidate = connection.getsockname()[0]
    except OSError:
        return None
    if not candidate or candidate.startswith('127.'):
        return None
    return candidate


def _wsl_windows_host_ip() -> Optional[str]:
    local_ip = _local_ipv4_address()
    if local_ip:
        return local_ip
    if not _wsl_available():
        return None
    try:
        result = subprocess.run(
            ['wsl', 'bash', '-lc', "awk '/nameserver/ {print $2; exit}' /etc/resolv.conf"],
            check=True,
            capture_output=True,
            text=True,
        )
    except Exception:
        return None
    candidate = result.stdout.strip()
    if not candidate:
        return None
    try:
        socket.inet_aton(candidate)
    except OSError:
        return None
    return candidate


def _wsl_guest_ip() -> Optional[str]:
    if not _wsl_available():
        return None
    try:
        result = subprocess.run(
            ['wsl', 'bash', '-lc', "hostname -I | awk '{print $1}'"],
            check=True,
            capture_output=True,
            text=True,
        )
    except Exception:
        return None
    candidate = result.stdout.strip().split()[0] if result.stdout.strip() else ''
    if not candidate:
        return None
    try:
        socket.inet_aton(candidate)
    except OSError:
        return None
    return candidate


def _from_wsl_path(path_value: str) -> Path:
    normalized = path_value.replace('\\', '/').strip()
    if normalized.startswith('/mnt/') and len(normalized) > 6:
        drive = normalized[5].upper()
        remainder = normalized[7:]
        return Path(f'{drive}:/{remainder}')
    return Path(normalized)


def _normalize_container_path(path_value: str) -> str:
    if not path_value:
        return path_value

    normalized = os.path.expandvars(os.path.expanduser(path_value.strip())).replace('\\', '/')
    if normalized.startswith('/'):
        return normalized
    if _wsl_available() and len(normalized) >= 2 and normalized[1] == ':':
        drive = normalized[0].lower()
        remainder = normalized[2:]
        if remainder.startswith('/'):
            remainder = remainder[1:]
        return f'/mnt/{drive}/{remainder}'
    return normalized


def _tool_image_definition(workspace_root: Path, tool_key: str) -> Optional[Tuple[str, Path]]:
    definitions = {
        'main_html': ('main_html.simg', workspace_root / 'Singularity.def'),
        'can_kpi': ('can_kpi.simg', workspace_root / 'KPI' / 'can_kpi' / 'can_singularity_KPI.def'),
        'udp_kpi': ('udp_kpi.simg', workspace_root / 'KPI' / 'UDP_KPI' / 'Singularity_KPI.def'),
        'interactive_plot': ('interactiveplot.simg', workspace_root / 'KPI' / 'intplot_kpi' / 'singularity_interactiveplot.def'),
        'rag': ('rag.simg', workspace_root / 'rag' / 'Singularity_RAG.def'),
    }
    return definitions.get(tool_key)


def _resolve_image_path(workspace_root: Path, tool_key: str, configured_image_path: str) -> Optional[Path]:
    configured = (configured_image_path or '').strip()
    candidate: Optional[Path] = None
    if configured:
        if os.name == 'nt' and configured.startswith('/mnt/'):
            candidate = _from_wsl_path(configured)
        else:
            raw_candidate = Path(configured)
            candidate = raw_candidate if raw_candidate.is_absolute() else workspace_root / raw_candidate
        if candidate.exists():
            return candidate.resolve()

    image_definition = _tool_image_definition(workspace_root, tool_key)
    if not image_definition:
        return None

    image_name, definition_path = image_definition
    return ensure_image(workspace_root, image_name, definition_path)


def _container_run_command(image_path: Path, arguments: List[str], env_overrides: Optional[Dict[str, str]] = None) -> List[str]:
    env_overrides = {key: str(value) for key, value in (env_overrides or {}).items() if value not in (None, '')}
    if _wsl_available():
        command_parts: List[str] = []
        if env_overrides:
            command_parts.append('env')
            for key, value in env_overrides.items():
                command_parts.append(f'APPTAINERENV_{key}={shlex.quote(value)}')
        command_parts.extend([
            'apptainer',
            'run',
            '--bind',
            '/mnt/c:/mnt/c',
            shlex.quote(_to_wsl_path(image_path)),
        ])
        command_parts.extend(shlex.quote(argument) for argument in arguments)
        return ['wsl', 'bash', '-lc', ' '.join(command_parts)]

    command = [_runtime_binary_name(), 'run']
    if Path('/mnt/c').exists():
        command.extend(['--bind', '/mnt/c:/mnt/c'])
    for key, value in env_overrides.items():
        command.extend(['--env', f'{key}={value}'])
    command.append(str(image_path))
    command.extend(arguments)
    return command


def _service_environment(workspace_root: Path, tool_key: str) -> Dict[str, str]:
    runtime_state_root = workspace_root / 'simg_sh_hpcc' / 'runtime_state'
    if tool_key == 'main_html':
        cache_dir = runtime_state_root / 'main_html' / 'cache_html'
        cache_dir.mkdir(parents=True, exist_ok=True)
        runtime_db = cache_dir / 'hpc_tools_dev.db'
        normalized_cache_dir = _normalize_container_path(str(cache_dir))
        normalized_runtime_db = _normalize_container_path(str(runtime_db))
        return {
            'HPCC_BROKER_HOST': os.environ.get('HPCC_BROKER_HOST', '127.0.0.1'),
            'HPCC_BROKER_PORT': os.environ.get('HPCC_BROKER_PORT', '9100'),
            'RAG_SERVICE_URL': os.environ.get('RAG_SERVICE_URL', 'http://127.0.0.1:5100'),
            'HOST': os.environ.get('HOST', '0.0.0.0'),
            'PORT': os.environ.get('PORT', '5001'),
            'CACHE_HTML_DIR': normalized_cache_dir,
            'HPCC_RUNTIME_DB': normalized_runtime_db,
            'DATABASE_URL': f'sqlite:///{normalized_runtime_db}',
        }
    if tool_key == 'rag':
        rag_state_dir = runtime_state_root / 'rag'
        vector_store_dir = rag_state_dir / 'vector_store'
        rag_state_dir.mkdir(parents=True, exist_ok=True)
        vector_store_dir.mkdir(parents=True, exist_ok=True)
        html_root = os.environ.get('HPCC_RAG_HTML_ROOT', '').strip() or _normalize_container_path(str(workspace_root))
        return {
            'FLASK_HOST': os.environ.get('FLASK_HOST', '0.0.0.0'),
            'FLASK_PORT': os.environ.get('FLASK_PORT', '5100'),
            'HTML_ROOT_PATH': html_root,
            'ALLOW_ALL_HTML_FALLBACK': os.environ.get('ALLOW_ALL_HTML_FALLBACK', 'true'),
            'RAG_AUTO_INGEST_ON_START': os.environ.get('RAG_AUTO_INGEST_ON_START', 'true'),
            'RAG_DATA_DIR': _normalize_container_path(str(rag_state_dir)),
            'RAG_SQLITE_PATH': _normalize_container_path(str(rag_state_dir / 'rag_logs.db')),
            'RAG_VECTOR_STORE_DIR': _normalize_container_path(str(vector_store_dir)),
            'VECTOR_STORE_JSON_PATH': _normalize_container_path(str(vector_store_dir / 'vector_store.json')),
        }
    return {}


def ensure_image(workspace_root: Path, image_name: str, definition_path: Path) -> Optional[Path]:
    image_path = workspace_root / 'simg_sh_hpcc' / image_name
    if image_path.exists():
        return image_path

    print(f'[{image_name}] image not found. Attempting build from {definition_path.name}...')
    try:
        image_path.parent.mkdir(parents=True, exist_ok=True)
        if os.name == 'nt' and shutil.which('wsl'):
            subprocess.run(
                [
                    'wsl',
                    'bash',
                    '-lc',
                    (
                        f"cd {shlex.quote(_to_wsl_path(workspace_root))} && "
                        f"apptainer build --fakeroot {shlex.quote(_to_wsl_path(image_path))} {shlex.quote(_to_wsl_path(definition_path))}"
                    ),
                ],
                check=True,
            )
        elif shutil.which('apptainer') or shutil.which('singularity'):
            subprocess.run(
                [_runtime_binary_name(), 'build', '--fakeroot', str(image_path), str(definition_path)],
                check=True,
                cwd=str(workspace_root),
            )
        else:
            return None
    except Exception as exc:
        print(f'[{image_name}] build failed: {exc}')
        return None

    return image_path if image_path.exists() else None


def default_service_command(workspace_root: Path, tool_key: str) -> Optional[List[str]]:
    if tool_key == 'main_html':
        image_path = _resolve_image_path(workspace_root, tool_key, '')
        if image_path:
            return _container_run_command(image_path, [], _service_environment(workspace_root, 'main_html'))
        return [sys.executable, str(workspace_root / 'main_html' / 'app.py')]

    if tool_key == 'rag':
        image_path = _resolve_image_path(workspace_root, tool_key, '')
        if image_path:
            return _container_run_command(image_path, ['--talk'], _service_environment(workspace_root, 'rag'))
        return [sys.executable, str(workspace_root / 'rag' / 'main.py')]
    return None


def _start_wsl_broker(workspace_root: Path, port: int) -> subprocess.Popen:
    command = (
        f"cd {shlex.quote(_to_wsl_path(workspace_root))} && "
        f"python3 hpcc_main.py --broker-only --host 0.0.0.0 --port {port}"
    )
    return subprocess.Popen(
        ['wsl', 'bash', '-lc', command],
        cwd=str(workspace_root),
    )


class PortForwardHandler(socketserver.BaseRequestHandler):
    def handle(self) -> None:
        try:
            upstream = socket.create_connection(self.server.target_address, timeout=5)
        except OSError:
            return

        stop_event = threading.Event()

        def _pipe(source: socket.socket, destination: socket.socket) -> None:
            try:
                while not stop_event.is_set():
                    chunk = source.recv(64 * 1024)
                    if not chunk:
                        break
                    destination.sendall(chunk)
            except OSError:
                pass
            finally:
                stop_event.set()
                try:
                    destination.shutdown(socket.SHUT_WR)
                except OSError:
                    pass

        forward = threading.Thread(target=_pipe, args=(self.request, upstream), daemon=True)
        backward = threading.Thread(target=_pipe, args=(upstream, self.request), daemon=True)
        forward.start()
        backward.start()
        forward.join()
        backward.join()
        try:
            upstream.close()
        except OSError:
            pass


class PortForwardServer(socketserver.ThreadingTCPServer):
    allow_reuse_address = True
    daemon_threads = True

    def __init__(self, address: Tuple[str, int], target_address: Tuple[str, int]):
        super().__init__(address, PortForwardHandler)
        self.target_address = target_address


def _start_windows_forwarders(remote_host: str, ports: List[int]) -> List[PortForwardServer]:
    forwarders: List[PortForwardServer] = []
    started_threads: List[threading.Thread] = []
    try:
        for port in sorted(set(ports)):
            server = PortForwardServer(('127.0.0.1', port), (remote_host, port))
            thread = threading.Thread(target=server.serve_forever, daemon=True)
            thread.start()
            forwarders.append(server)
            started_threads.append(thread)
    except Exception:
        for server in forwarders:
            server.shutdown()
            server.server_close()
        raise
    return forwarders


def _stop_windows_forwarders(forwarders: List[PortForwardServer]) -> None:
    for server in forwarders:
        try:
            server.shutdown()
            server.server_close()
        except OSError:
            pass


class RuntimeBroker:
    def __init__(self, workspace_root: Path, store: RuntimeStore):
        self.workspace_root = workspace_root
        self.store = store
        self.processes: Dict[int, subprocess.Popen] = {}
        self.lock = threading.Lock()

    def handle(self, payload: Dict[str, Any]) -> Dict[str, Any]:
        action = (payload.get('action') or '').strip().lower()
        if action == 'ping':
            return {
                'ok': True,
                'status': 'alive',
                'timestamp': self._utcnow(),
                'tools': self.store.list_tools(),
            }
        if action == 'status':
            runtime_job_id = int(payload['runtime_job_id'])
            job = self.store.get_job(runtime_job_id)
            return {'ok': True, 'job': job}
        if action == 'cancel':
            return self._cancel(int(payload['runtime_job_id']))
        if action == 'submit':
            return self._submit(payload)
        return {'ok': False, 'error': f'Unsupported action: {action}'}

    def _submit(self, payload: Dict[str, Any]) -> Dict[str, Any]:
        tool_key = (payload.get('tool_key') or '').strip()
        if not tool_key:
            return {'ok': False, 'error': 'tool_key is required'}

        tool = self.store.get_tool(tool_key)
        if not tool:
            return {'ok': False, 'error': f'Unknown tool: {tool_key}'}

        spec = self._build_spec(tool, payload)
        runtime_job_id = self.store.create_job(
            tool_key=tool_key,
            requested_by=payload.get('user', 'unknown'),
            session_id=payload.get('session_id', ''),
            mode=spec['mode'],
            input_path=spec['input_path'],
            output_path=spec['output_path'],
            log_path=spec['log_path'],
            command=spec['command'],
            resources=spec['resources'],
            request_payload=payload,
            status='QUEUED',
        )
        self.store.append_event(runtime_job_id, 'info', f'Prepared command for {tool_key}')
        self._launch(runtime_job_id, spec)
        return {
            'ok': True,
            'job_id': runtime_job_id,
            'status': 'QUEUED',
            'log_path': spec['log_path'],
            'output_path': spec['output_path'],
            'command': spec['command'],
        }

    def _launch(self, runtime_job_id: int, spec: Dict[str, Any]) -> None:
        os.makedirs(Path(spec['log_path']).parent, exist_ok=True)
        log_fp = open(spec['log_path'], 'a', encoding='utf-8', errors='replace')
        log_fp.write('COMMAND: ' + ' '.join(spec['command']) + '\n')
        log_fp.write('START: ' + self._utcnow() + '\n')
        log_fp.write('CWD: ' + spec['cwd'] + '\n\n')
        log_fp.flush()

        creationflags = 0
        popen_kwargs: Dict[str, Any] = {'start_new_session': True}
        if os.name == 'nt':
            creationflags = subprocess.CREATE_NEW_PROCESS_GROUP
            popen_kwargs.pop('start_new_session', None)

        process = subprocess.Popen(
            spec['command'],
            cwd=spec['cwd'],
            env=spec['env'],
            stdout=log_fp,
            stderr=subprocess.STDOUT,
            text=True,
            creationflags=creationflags,
            **popen_kwargs,
        )
        with self.lock:
            self.processes[runtime_job_id] = process
        self.store.update_job(
            runtime_job_id,
            status='RUNNING',
            pid=process.pid,
            started_at=self._utcnow(),
        )
        self.store.append_event(runtime_job_id, 'info', f'Launched PID {process.pid}')

        watcher = threading.Thread(
            target=self._watch_process,
            args=(runtime_job_id, process, log_fp),
            daemon=True,
        )
        watcher.start()

    def _watch_process(self, runtime_job_id: int, process: subprocess.Popen, log_fp) -> None:
        return_code = process.wait()
        log_fp.write('\nEND: ' + self._utcnow() + '\n')
        log_fp.write('RETURN_CODE: ' + str(return_code) + '\n')
        log_fp.flush()
        log_fp.close()
        status = 'COMPLETED' if return_code == 0 else 'FAILED'
        self.store.update_job(
            runtime_job_id,
            status=status,
            return_code=return_code,
            completed_at=self._utcnow(),
        )
        self.store.append_event(runtime_job_id, 'info', f'Process finished with code {return_code}')
        with self.lock:
            self.processes.pop(runtime_job_id, None)

    def _cancel(self, runtime_job_id: int) -> Dict[str, Any]:
        with self.lock:
            process = self.processes.get(runtime_job_id)
        if not process:
            job = self.store.get_job(runtime_job_id)
            return {'ok': True, 'job': job, 'message': 'Job is not running'}

        if os.name == 'nt':
            process.terminate()
        else:
            os.killpg(process.pid, signal.SIGTERM)

        self.store.update_job(
            runtime_job_id,
            status='CANCELLED',
            completed_at=self._utcnow(),
            error_message='Cancelled by user',
        )
        self.store.append_event(runtime_job_id, 'warning', 'Cancelled by user request')
        return {'ok': True, 'job': self.store.get_job(runtime_job_id)}

    def _build_spec(self, tool: Dict[str, Any], payload: Dict[str, Any]) -> Dict[str, Any]:
        tool_key = tool['tool_key']
        paths = payload.get('paths', {}) or {}
        resources = payload.get('resources', {}) or {}
        requested_by = payload.get('user', 'unknown')
        timestamp = datetime.now(timezone.utc).strftime('%Y%m%d_%H%M%S')
        run_dir = self.workspace_root / 'simg_sh_hpcc' / 'runs' / requested_by / f'{tool_key}_{timestamp}'
        run_dir.mkdir(parents=True, exist_ok=True)
        output_path = paths.get('output_dir') or str(run_dir / 'output')
        os.makedirs(output_path, exist_ok=True)
        log_path = str(run_dir / 'execution.log')
        command, mode, input_path = self._tool_command(tool_key, tool, paths, output_path)
        command = self._maybe_prefix_scheduler(command, resources)

        return {
            'command': command,
            'cwd': str(self.workspace_root),
            'env': os.environ.copy(),
            'mode': mode,
            'input_path': input_path,
            'output_path': output_path,
            'log_path': log_path,
            'resources': resources,
        }

    def _tool_command(
        self,
        tool_key: str,
        tool: Dict[str, Any],
        paths: Dict[str, Any],
        output_path: str,
    ) -> Tuple[List[str], str, str]:
        image_path = _resolve_image_path(self.workspace_root, tool_key, tool.get('image_path', ''))
        input_mode = (paths.get('input_mode') or 'json').strip().lower()
        runtime_output_path = _normalize_container_path(output_path)

        if tool_key != 'hyperlink' and image_path is None:
            raise ValueError(f'Container image is not available for {tool_key}')

        if tool_key in {'can_kpi', 'udp_kpi'}:
            if input_mode == 'hdf':
                input_hdf = _normalize_container_path(paths.get('input_hdf', '').strip())
                output_hdf = _normalize_container_path(paths.get('output_hdf', '').strip())
                if not input_hdf or not output_hdf:
                    raise ValueError('input_hdf and output_hdf are required for HDF mode')
                return _container_run_command(image_path, ['hdf', input_hdf, output_hdf, runtime_output_path]), 'hdf', input_hdf
            json_path = _normalize_container_path(paths.get('json_path', '').strip())
            if not json_path:
                raise ValueError('json_path is required for JSON mode')
            return _container_run_command(image_path, ['json', json_path, runtime_output_path]), 'json', json_path

        if tool_key == 'interactive_plot':
            optional_config = _normalize_container_path(paths.get('optional_config', '').strip())
            if input_mode == 'hdf':
                input_hdf = _normalize_container_path(paths.get('input_hdf', '').strip())
                output_hdf = _normalize_container_path(paths.get('output_hdf', '').strip())
                if not input_hdf or not output_hdf:
                    raise ValueError('input_hdf and output_hdf are required for HDF mode')
                command = [input_hdf, output_hdf, runtime_output_path]
                if optional_config:
                    command.append(optional_config)
                return _container_run_command(image_path, command), 'hdf', input_hdf
            config_xml = _normalize_container_path(paths.get('config_xml', '').strip())
            inputs_json = _normalize_container_path(paths.get('json_path', '').strip())
            if not config_xml or not inputs_json:
                raise ValueError('config_xml and json_path are required for interactive plot JSON mode')
            command = [config_xml, inputs_json, runtime_output_path]
            if optional_config:
                command.append(optional_config)
            return _container_run_command(image_path, command), 'json', inputs_json

        if tool_key == 'rag':
            html_root = _normalize_container_path(paths.get('html_root', '').strip()) or _normalize_container_path(str(self.workspace_root / 'main_html'))
            if paths.get('ingest_only'):
                env_overrides = _service_environment(self.workspace_root, 'rag')
                env_overrides['HTML_ROOT_PATH'] = html_root
                return _container_run_command(image_path, ['--scrap', html_root], env_overrides), 'ingest', html_root
            env_overrides = _service_environment(self.workspace_root, 'rag')
            env_overrides['HTML_ROOT_PATH'] = html_root
            return _container_run_command(image_path, ['--talk'], env_overrides), 'service', html_root

        if tool_key == 'main_html':
            return _container_run_command(image_path, [], _service_environment(self.workspace_root, 'main_html')), 'service', ''

        if tool_key == 'hyperlink':
            html_root = paths.get('html_root', '').strip()
            video_root = paths.get('video_root', '').strip()
            script = self.workspace_root / 'Hyperlink_tool' / 'code' / 'main.py'
            command = [sys.executable, str(script)]
            if html_root:
                command.append(html_root)
            if video_root:
                command.append(video_root)
            return command, 'viewer', html_root

        raise ValueError(f'Unsupported tool command for {tool_key}')

    @staticmethod
    def _maybe_prefix_scheduler(command: List[str], resources: Dict[str, Any]) -> List[str]:
        use_slurm = str(resources.get('scheduler', '')).lower() == 'slurm'
        if not use_slurm:
            return command
        if shutil.which('srun') is None:
            return command

        prefixed = [
            'srun',
            '--exclusive',
            '-N', '1',
            '-n', '1',
        ]
        if resources.get('partition'):
            prefixed.append(f"--partition={resources['partition']}")
        if resources.get('memory'):
            prefixed.append(f"--mem={resources['memory']}")
        if resources.get('cpus'):
            prefixed.append(f"--cpus-per-task={resources['cpus']}")
        if resources.get('time_limit'):
            prefixed.append(f"--time={resources['time_limit']}")
        return prefixed + command

    @staticmethod
    def _utcnow() -> str:
        return datetime.now(timezone.utc).isoformat().replace('+00:00', 'Z')


class BrokerHandler(socketserver.StreamRequestHandler):
    def handle(self) -> None:
        raw = self.rfile.readline()
        if not raw:
            return
        try:
            payload = json.loads(raw.decode('utf-8'))
            response = self.server.runtime_broker.handle(payload)
        except Exception as exc:
            response = {'ok': False, 'error': str(exc)}
        self.wfile.write((json.dumps(response) + '\n').encode('utf-8'))


class ThreadedBrokerServer(socketserver.ThreadingTCPServer):
    allow_reuse_address = True

    def __init__(self, address: Tuple[str, int], handler_cls, runtime_broker: RuntimeBroker):
        super().__init__(address, handler_cls)
        self.runtime_broker = runtime_broker


def maybe_start_main_html(ui_command: str, launch_env: Dict[str, str]) -> Optional[subprocess.Popen]:
    workspace_root = Path(__file__).resolve().parent
    command: Optional[List[str]] = None
    if ui_command:
        command = shlex.split(ui_command)
    else:
        command = default_service_command(workspace_root, 'main_html')

    if not command:
        return None
    return subprocess.Popen(command, cwd=str(workspace_root), env=launch_env)


def maybe_start_rag(launch_env: Dict[str, str]) -> Optional[subprocess.Popen]:
    auto_start = (os.environ.get('HPCC_AUTO_START_RAG') or '1').strip().lower()
    if auto_start not in {'1', 'true', 'yes', 'y'}:
        return None

    workspace_root = Path(__file__).resolve().parent
    command = default_service_command(workspace_root, 'rag')
    if not command:
        return None
    return subprocess.Popen(command, cwd=str(workspace_root), env=launch_env)


def main() -> None:
    parser = argparse.ArgumentParser(description='HPCC runtime broker for main_html + SIMG tools')
    parser.add_argument('--host', default=os.environ.get('HPCC_BROKER_HOST', '127.0.0.1'))
    parser.add_argument('--port', type=int, default=int(os.environ.get('HPCC_BROKER_PORT', '9100')))
    parser.add_argument('--ui-command', default=os.environ.get('HPCC_MAIN_HTML_CMD', ''))
    parser.add_argument('--broker-only', action='store_true', help='Run the socket broker only and skip launching services')
    args = parser.parse_args()

    workspace_root = Path(__file__).resolve().parent
    store = RuntimeStore()
    store.ensure_defaults()
    runtime_broker = RuntimeBroker(workspace_root, store)

    if _wsl_available() and os.name == 'nt' and not args.broker_only:
        launch_env = os.environ.copy()
        launch_env['HPCC_BROKER_HOST'] = '127.0.0.1'
        launch_env['HPCC_BROKER_PORT'] = str(args.port)
        launch_env.setdefault('RAG_SERVICE_URL', 'http://127.0.0.1:5100')
        launch_env.setdefault('HOST', '0.0.0.0')
        launch_env.setdefault('PORT', '5001')
        launch_env.setdefault('FLASK_HOST', '0.0.0.0')
        launch_env.setdefault('FLASK_PORT', '5100')

        broker_process = _start_wsl_broker(workspace_root, args.port)
        time.sleep(2)
        wsl_service_host = _wsl_guest_ip()
        forwarders: List[PortForwardServer] = []
        if wsl_service_host:
            forwarders = _start_windows_forwarders(
                wsl_service_host,
                [args.port, int(launch_env['PORT']), int(launch_env['FLASK_PORT'])],
            )
        ui_process = maybe_start_main_html(args.ui_command, launch_env)
        rag_process = maybe_start_rag(launch_env)

        try:
            print(f'HPCC broker delegated to WSL on 127.0.0.1:{args.port}')
            print(f'HPCC broker launcher PID {broker_process.pid}')
            if wsl_service_host:
                print(f'Windows localhost forwarding active via WSL {wsl_service_host}')
            if ui_process is not None:
                print(f'main_html launched with PID {ui_process.pid}')
            if rag_process is not None:
                print(f'rag launched with PID {rag_process.pid}')
            while True:
                if broker_process.poll() is not None:
                    raise RuntimeError('WSL broker process exited unexpectedly')
                time.sleep(1)
        except KeyboardInterrupt:
            pass
        finally:
            if ui_process is not None and ui_process.poll() is None:
                ui_process.terminate()
            if rag_process is not None and rag_process.poll() is None:
                rag_process.terminate()
            if broker_process.poll() is None:
                broker_process.terminate()
            _stop_windows_forwarders(forwarders)
        return

    bind_host = args.host
    advertised_host = os.environ.get('HPCC_BROKER_ADVERTISE_HOST', '').strip()
    if not advertised_host:
        if _wsl_available():
            advertised_host = _wsl_windows_host_ip() or ''
            if advertised_host and bind_host == '127.0.0.1':
                bind_host = '0.0.0.0'
        if not advertised_host:
            advertised_host = '127.0.0.1' if bind_host in {'0.0.0.0', '::'} else bind_host
    launch_env = os.environ.copy()
    launch_env.setdefault('HPCC_BROKER_HOST', advertised_host)
    launch_env.setdefault('HPCC_BROKER_PORT', str(args.port))
    launch_env.setdefault('RAG_SERVICE_URL', 'http://127.0.0.1:5100')
    launch_env.setdefault('HOST', '0.0.0.0')
    launch_env.setdefault('PORT', '5001')
    launch_env.setdefault('FLASK_HOST', '0.0.0.0')
    launch_env.setdefault('FLASK_PORT', '5100')
    os.environ.update({
        'HPCC_BROKER_HOST': launch_env['HPCC_BROKER_HOST'],
        'HPCC_BROKER_PORT': launch_env['HPCC_BROKER_PORT'],
        'RAG_SERVICE_URL': launch_env['RAG_SERVICE_URL'],
        'HOST': launch_env['HOST'],
        'PORT': launch_env['PORT'],
        'FLASK_HOST': launch_env['FLASK_HOST'],
        'FLASK_PORT': launch_env['FLASK_PORT'],
    })

    ui_process = None if args.broker_only else maybe_start_main_html(args.ui_command, launch_env)
    rag_process = None if args.broker_only else maybe_start_rag(launch_env)

    with ThreadedBrokerServer((bind_host, args.port), BrokerHandler, runtime_broker) as server:
        try:
            print(f'HPCC broker listening on {bind_host}:{args.port}')
            if advertised_host != bind_host:
                print(f'HPCC broker advertised to containers as {advertised_host}:{args.port}')
            if ui_process is not None:
                print(f'main_html launched with PID {ui_process.pid}')
            if rag_process is not None:
                print(f'rag launched with PID {rag_process.pid}')
            server.serve_forever()
        finally:
            if ui_process is not None and ui_process.poll() is None:
                ui_process.terminate()
            if rag_process is not None and rag_process.poll() is None:
                rag_process.terminate()


if __name__ == '__main__':
    main()