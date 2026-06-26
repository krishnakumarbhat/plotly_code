import os, platform, shutil, subprocess, sys, json, hashlib, socket, threading, time
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parent
GEN = ROOT / 'generate_upload'
META = ROOT / '.metadata.json'
BUILD_ORDER = ['main_html.simg', 'rag.simg', 'kpi/can/can_kpi.simg', 'kpi/udp/udp_kpi.simg', 'kpi/int_plot/intplot_kpi.simg']

DEF_MAP = {
    'main_html.simg':          ROOT / 'Singularity.def',
    'rag.simg':                ROOT / 'rag' / 'Singularity_RAG.def',
    'kpi/can/can_kpi.simg':    ROOT / 'KPI' / 'can_kpi' / 'can_singularity_KPI.def',
    'kpi/udp/udp_kpi.simg':    ROOT / 'KPI' / 'UDP_KPI' / 'Singularity_KPI.def',
    'kpi/int_plot/intplot_kpi.simg': ROOT / 'KPI' / 'intplot_kpi' / 'singularity_interactiveplot.def',
}

SIMGG_SRC = {
    'main_html.simg':          [ROOT / 'Singularity.def', ROOT / 'main_html', ROOT / 'Hyperlink_tool', ROOT / 'KPI'],
    'rag.simg':                [ROOT / 'rag' / 'Singularity_RAG.def', ROOT / 'rag'],
    'kpi/can/can_kpi.simg':    [ROOT / 'KPI' / 'can_kpi' / 'can_singularity_KPI.def', ROOT / 'KPI' / 'can_kpi'],
    'kpi/udp/udp_kpi.simg':    [ROOT / 'KPI' / 'UDP_KPI' / 'Singularity_KPI.def', ROOT / 'KPI' / 'UDP_KPI'],
    'kpi/int_plot/intplot_kpi.simg': [ROOT / 'KPI' / 'intplot_kpi' / 'singularity_interactiveplot.def', ROOT / 'KPI' / 'intplot_kpi'],
}

SCRIPTS = ['bundle_common.sh', 'cleanup_memory.sh', 'kpi_runtime_launcher.sh']
BUNDLE_DIRS = ['main_html', 'Hyperlink_tool', 'KPI']


print('generate_upload.py starting...', flush=True)


def _auto_wsl():
    if platform.system() != 'Windows':
        return False
    # Only re-execute in WSL for 'generate' (needs apptainer)
    # 'upload' and 'deploy' run natively on Windows (has paramiko)
    if 'generate' not in sys.argv:
        return False
    if shutil.which('apptainer') or shutil.which('singularity'):
        return False
    if os.environ.get('GEN_NO_WSL') == '1':
        return False
    wsl = shutil.which('wsl') or shutil.which('wsl.exe')
    if not wsl:
        return False
    try:
        result = subprocess.run([wsl, '-d', 'Ubuntu', '--', 'true'],
                                capture_output=True, timeout=30)
        if result.returncode != 0:
            raise RuntimeError('Ubuntu distro not available')
    except Exception:
        print('WSL found but Ubuntu distro not available, running natively', flush=True)
        return False
    drive = ROOT.drive[0].lower()
    wsl_path = f'/mnt/{drive}{str(ROOT)[2:].replace(chr(92), "/")}/{Path(__file__).name}'
    cmd = [wsl, '-d', 'Ubuntu', '--', 'python3', wsl_path] + sys.argv[1:]
    print(f'Re-executing in WSL: {" ".join(cmd)}', flush=True)
    sys.exit(subprocess.call(cmd))


def _run_in_wsl(args=None):
    """Run generate_upload.py in WSL with given args (e.g. ['generate'])."""
    wsl = shutil.which('wsl') or shutil.which('wsl.exe')
    if not wsl:
        return False
    drive = ROOT.drive[0].lower()
    wsl_path = f'/mnt/{drive}{str(ROOT)[2:].replace(chr(92), "/")}/{Path(__file__).name}'
    cmd = [wsl, '-d', 'Ubuntu', '--', 'python3', wsl_path] + (args or ['generate'])
    print(f'Running in WSL: {" ".join(cmd)}', flush=True)
    return subprocess.call(cmd) == 0


_auto_wsl()


def _sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(65536), b''):
            h.update(chunk)
    return h.hexdigest()


def _rel_key(path) -> str:
    if isinstance(path, Path):
        return path.as_posix()
    return str(path).replace('\\', '/')


def _read_meta():
    if META.exists():
        try:
            return json.loads(META.read_text())
        except Exception:
            pass
    return {'build': {}, 'upload': {}}


def _save_meta(meta):
    META.write_text(json.dumps(meta, indent=2, sort_keys=True))


_SRC_EXTS = {'.py', '.html', '.htm', '.js', '.css', '.scss', '.def', '.sh', '.json', '.xml', '.yaml', '.yml', '.txt', '.cfg', '.env', '.md', '.spec', '.toml', '.jinja', '.jinja2'}
_DATA_EXTS = {'.h5', '.hdf5', '.gguf', '.mf4', '.csv', '.pyc', '.pyd', '.pyo', '.so', '.log', '.db', '.sqlite', '.simg', '.sif', '.zip', '.tar', '.gz', '.tar.gz', '.tgz', '.bin', '.pt', '.pth', '.onnx', '.npy', '.npz', '.wav', '.mp3', '.mp4', '.avi', '.png', '.jpg', '.jpeg', '.gif', '.ico', '.svg', '.woff', '.woff2', '.ttf', '.eot', '.pdf', '.o', '.a', '.lib', '.dll', '.dylib', '.exe'}

def _is_source_file(f: Path) -> bool:
    return f.suffix.lower() in _SRC_EXTS or (f.suffix.lower() not in _DATA_EXTS and not f.name.startswith('.'))

def _deps_hash(paths):
    h = hashlib.sha256()
    for p in paths:
        if p.is_file():
            if _is_source_file(p):
                h.update(p.read_bytes())
        elif p.is_dir():
            for f in sorted(p.rglob('*')):
                if f.is_file() and _is_source_file(f):
                    h.update(f.read_bytes())
    return h.hexdigest()


def _check_runtime():
    return shutil.which('apptainer') or shutil.which('singularity') or ''


def build_simg(img_rel):
    dst = GEN / img_rel
    meta = _read_meta()
    deps = SIMGG_SRC.get(img_rel, [])
    current_dep_hash = _deps_hash(deps)
    prev_dep_hash = meta.get('build', {}).get(img_rel, {}).get('deps_hash', '')
    if dst.exists() and current_dep_hash == prev_dep_hash:
        print(f'  up-to-date: {img_rel}')
        return False
    def_path = DEF_MAP.get(img_rel)
    if not def_path or not def_path.exists():
        alt = ROOT / 'Singularity.def'
        def_path = alt if alt.exists() else None
    if not def_path:
        print(f'  ERROR: no .def for {img_rel}')
        raise SystemExit(1)
    print(f'  building {img_rel}...')
    dst.parent.mkdir(parents=True, exist_ok=True)
    runtime = _check_runtime()
    if not runtime:
        print(f'  ERROR: apptainer/singularity not found — cannot build {img_rel}')
        print(f'  Run this command from WSL (Ubuntu) where apptainer is installed.')
        print(f'  Or install WSL: wsl --install -d Ubuntu')
        raise SystemExit(1)
    build_env = os.environ.copy()
    build_env['APPTAINER_TMPDIR'] = '/tmp'
    # Build in native Linux /tmp to avoid mksquashfs crash on /mnt/c/ (Windows 9P mount)
    import tempfile as _tf
    with _tf.TemporaryDirectory(dir='/tmp') as _build_dir:
        _tmp_img = Path(_build_dir) / dst.name
        subprocess.run(
            [runtime, 'build', '--fakeroot', '--tmpdir', '/tmp', str(_tmp_img), str(def_path)],
            check=True, env=build_env, cwd=str(ROOT),
        )
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(_tmp_img, dst)
    print(f'  done: {img_rel}')
    meta.setdefault('build', {})[img_rel] = {'deps_hash': current_dep_hash}
    _save_meta(meta)
    return True


def build_pyz():
    src = ROOT / 'main_html' / 'temp_dir' / 'hpcc_main.py'
    dst = GEN / 'hpcc_main.pyz'
    dst_py = GEN / 'hpcc_main.py'
    meta = _read_meta()
    current_hash = _sha256(src) if src.exists() else ''
    prev_hash = meta.get('build', {}).get('hpcc_main.pyz', {}).get('src_hash', '')
    if dst.exists() and current_hash and current_hash == prev_hash:
        print(f'  up-to-date: hpcc_main.pyz')
        return
    print('  building hpcc_main.pyz...')
    dst.parent.mkdir(parents=True, exist_ok=True)
    try:
        # Build a real ZIP archive compatible with Python 3.6-3.13+
        _build_compatible_zipapp(src, dst)
        print('  built hpcc_main.pyz')
    except Exception as exc:
        print(f'  zipapp build failed ({exc}), falling back to plain .py')
        shutil.copy2(src, dst_py)
        print('  copied hpcc_main.py (fallback)')
    meta.setdefault('build', {})['hpcc_main.pyz'] = {'src_hash': current_hash}
    _save_meta(meta)


def _build_compatible_zipapp(src: Path, dst: Path):
    import zipfile, io
    dst.parent.mkdir(parents=True, exist_ok=True)
    # Build a proper ZIP archive with __main__.py
    buf = io.BytesIO()
    with zipfile.ZipFile(buf, 'w', zipfile.ZIP_DEFLATED) as z:
        z.write(str(src), '__main__.py')
    zip_data = buf.getvalue()
    # Write shebang + ZIP data (compatible with all Python 3 versions)
    shebang = b'#!/usr/bin/env python3\n'
    dst.write_bytes(shebang + zip_data)


def generate():
    runtime = _check_runtime()
    if not runtime:
        print('ERROR: apptainer/singularity is required')
        print('On Windows, run from WSL (Ubuntu) where apptainer is installed.')
        print('Or install: wsl --install -d Ubuntu')
        raise SystemExit(1)
    built_any = False
    for img_rel in BUILD_ORDER:
        if build_simg(img_rel):
            built_any = True
    if not built_any:
        print('  all simgs up-to-date')

    build_pyz()

    for s in SCRIPTS:
        src = ROOT / s
        if src.exists():
            shutil.copy2(src, GEN / s)
            print(f'  copied {s}')

    for name in BUNDLE_DIRS:
        src = ROOT / name
        dst = GEN / 'bundle_src' / name
        if src.is_dir():
            if dst.exists():
                shutil.rmtree(dst)
            shutil.copytree(src, dst, ignore=shutil.ignore_patterns('__pycache__', '*.pyc', '.git'))
            print(f'  copied bundle_src/{name}/')

    if (ROOT / 'rag').is_dir():
        dst = GEN / 'rag'
        if dst.exists():
            shutil.rmtree(dst)
        shutil.copytree(ROOT / 'rag', dst, ignore=shutil.ignore_patterns('__pycache__', '*.pyc', 'model', 'tools', 'data'))
        print('  copied rag/ (app code only)')

    if (ROOT / 'resources.py').is_file():
        shutil.copy2(ROOT / 'resources.py', GEN / 'resources.py')

    if (ROOT / 'jira').is_dir():
        dst = GEN / 'jira'
        if dst.exists():
            shutil.rmtree(dst)
        shutil.copytree(ROOT / 'jira', dst, ignore=shutil.ignore_patterns('__pycache__', '*.pyc', '.git'))
        print('  copied jira/')

    _bundle_model_dir(ROOT / 'rag' / 'model', GEN / 'rag' / 'model', '*.gguf')

    for sub in ['db', 'logs', 'rag/vector_store']:
        (GEN / 'store' / sub).mkdir(parents=True, exist_ok=True)
    print('  created store/')

    _write_launcher()
    _fix_shell_scripts()
    _ensure_rag_sh()
    _write_readme()
    _verify_critical_files()
    print(f'Generate complete: {GEN}')


_CRITICAL_BUNDLE_FILES = [
    'bundle_src/Hyperlink_tool/code/html_online/static/viewer.html',
    'bundle_src/Hyperlink_tool/code/html_online/main.py',
    'bundle_src/main_html/app.py',
    'bundle_src/KPI',
    'run_hpcc.sh',
    'hpcc_main.pyz',
]

def _verify_critical_files() -> None:
    missing = []
    for rel in _CRITICAL_BUNDLE_FILES:
        if not (GEN / rel).exists():
            missing.append(rel)
    if missing:
        print('WARNING: critical files missing in output:')
        for path in missing:
            print(f'  MISSING: {path}')
        print('  Check that source files exist in the workspace before re-running.')
    else:
        print(f'  verified {len(_CRITICAL_BUNDLE_FILES)} critical files')


_MODEL_BINARY_EXT = {'.gguf', '.safetensors', '.bin', '.pt', '.pth'}

def _is_model_binary(path: Path) -> bool:
    return path.suffix in _MODEL_BINARY_EXT


def _bundle_model_dir(src: Path, dst: Path, pattern: str = '*'):
    if not src.exists():
        return
    key = str(dst.relative_to(GEN)) if dst.is_relative_to(GEN) else dst.name

    def _should_ignore(path: Path, base: Path) -> bool:
        for part in path.relative_to(base).parts:
            if part.startswith('.') or part in ('__pycache__',):
                return True
        return False

    if pattern == '*.gguf':
        for f in src.glob(pattern):
            if _should_ignore(f, src):
                continue
            rel = f.name
            target = dst / rel
            if target.exists():
                print(f'  model unchanged: rag/{key}/{rel}')
                continue
            dst.mkdir(parents=True, exist_ok=True)
            shutil.copy2(f, target)
            print(f'  copied model: rag/{key}/{rel}')
    else:
        files = sorted(src.rglob('*'))
        copied_any = False
        for f in files:
            if not f.is_file():
                continue
            if _should_ignore(f, src):
                continue
            if f.suffix in ('.pyc',):
                continue
            rel = f.relative_to(src).as_posix()
            target = dst / rel
            if target.exists():
                continue
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(f, target)
            print(f'  copied model: rag/{key}/{rel}')
            copied_any = True
        if not copied_any:
            print(f'  model unchanged: rag/{key}/')


def _dir_combined_hash(path: Path) -> str:
    h = hashlib.sha256()
    for f in sorted(path.rglob('*')):
        if f.is_file() and not f.name.startswith('.'):
            h.update(f.read_bytes())
    return h.hexdigest()


def _write_launcher():
    src = ROOT / 'main_hpcc.sh'
    if not src.exists():
        print('  WARNING: main_hpcc.sh not found')
        return
    content = src.read_text(encoding='utf-8')
    lines = content.splitlines()
    body = []
    for i, line in enumerate(lines):
        if i == 0 and line.strip().startswith('#!'):
            continue
        if line.strip().startswith('set -euo pipefail'):
            continue
        if line.strip().startswith('set -e'):
            continue
        if line.strip().startswith('SCRIPT_DIR=') and '"$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"' in line:
            continue
        body.append(line)
    launcher = f'''#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${{BASH_SOURCE[0]}}")" && pwd)"
LOG_PATH="$SCRIPT_DIR/run_hpcc.log"

echo "[$(date -Iseconds 2>/dev/null || date)] Starting HPCC bundle" | tee -a "$LOG_PATH"

{chr(10).join(body)}
'''
    (GEN / 'run_hpcc.sh').write_text(launcher, encoding='utf-8')
    (GEN / 'run_hpcc.sh').chmod(0o755)
    print('  wrote run_hpcc.sh')


def _fix_shell_scripts():
    count = 0
    for f in GEN.rglob('*.sh'):
        c = f.read_bytes()
        if b'\r\n' in c:
            f.write_bytes(c.replace(b'\r\n', b'\n'))
            count += 1
        st = f.stat()
        if not (st.st_mode & 0o111):
            f.chmod(st.st_mode | 0o111)
    if count:
        print(f'  fixed CRLF: {count} files')


def _ensure_rag_sh():
    rag_sh = GEN / 'rag' / 'run_rag.sh'
    if rag_sh.exists():
        rag_sh.chmod(0o755)
        return
    rag_sh.parent.mkdir(parents=True, exist_ok=True)
    rag_sh.write_text('#!/usr/bin/env bash\nset -euo pipefail\n# RAG auto-start placeholder\necho "RAG service not bundled"\nexit 0\n')
    rag_sh.chmod(0o755)
    print('  created rag/run_rag.sh (placeholder)')


def _write_readme():
    text = '''# HPCC Runtime Console - Deployment

## Structure
```
deploy_root/
├── run_hpcc.sh            <- Launcher
├── hpcc_main.pyz          <- Broker (TCP 9100)
├── main_html.simg         <- Flask web app (port 5002)
├── rag.simg               <- RAG service (port 5100)
├── kpi/
│   ├── can/can_kpi.simg
│   ├── udp/udp_kpi.simg
│   └── int_plot/intplot_kpi.simg
├── bundle_src/            <- Live source (bind-mounted)
├── store/                 <- Runtime data
├── resources.py           <- Resource allocation
├── jira/                  <- Jira integration
└── README_deploy.md
```

## Usage
1. `python generate_upload.py deploy` on machine with apptainer
2. Copy `generate_upload/` to cluster
3. `cd generate_upload && bash run_hpcc.sh`
4. Open `http://<cluster-ip>:5002/html`

## Resource Allocation
Edit `resources.py` to change partitions, accounts.
- Krakow: partition=plcyf-com, account=RNA-SDV-SRR7
- Southfield: partition=defq, account=radarcore

## RAG Service
- llama.cpp bundled inside rag.simg, GGUF model bind-mounted at runtime
- Port 5100, auto-starts with main_html
'''
    (GEN / 'README_deploy.md').write_text(text, encoding='utf-8')
    print('  wrote README_deploy.md')


def _save_upload_hashes():
    meta = _read_meta()
    meta.setdefault('upload', {})
    for f in GEN.rglob('*'):
        if f.is_dir() or f.name.startswith('.'):
            continue
        rel = _rel_key(f.relative_to(GEN))
        if _is_model_binary(f):
            meta['upload'][rel] = '__name_only__'
        else:
            meta['upload'][rel] = _sha256(f)
    _save_meta(meta)


def _load_env():
    env_path = ROOT / '.env'
    if not env_path.exists():
        return {}
    env = {}
    for line in env_path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        if '=' in line:
            k, v = line.split('=', 1)
            env[k.strip()] = v.strip().strip('"').strip("'")
    return env


def _sftp_ensure_dir(sftp, path, known_dirs=None):
    normalized = str(PurePosixPath(path))
    if known_dirs is not None and normalized in known_dirs:
        return
    parts = normalized.strip('/').split('/')
    for i in range(1, len(parts) + 1):
        p = '/' + '/'.join(parts[:i])
        if known_dirs is not None and p in known_dirs:
            continue
        try:
            sftp.stat(p)
        except FileNotFoundError:
            sftp.mkdir(p)
        if known_dirs is not None:
            known_dirs.add(p)


def upload():
    env = _load_env()
    if not env:
        print('ERROR: .env file not found or empty. Create simg_zmq/.env with:')
        print('  netid=your_netid')
        print('  netid_password=your_password')
        print('  krakow_path=/path/...')
        print('  southfield_path=/path/...')
        raise SystemExit(1)

    netid = env.get('netid', '')
    password = env.get('netid_password', '')
    krakow_path = env.get('krakow_path', '')
    southfield_path = env.get('southfield_path', '')
    host = env.get('host', '') or env.get('HOST', '')
    port = int(env.get('port', '22'))
    timeout_s = int(env.get('sftp_timeout', '120'))

    krakow_host = env.get('krakow_host', '') or env.get('KRAKOW_HOST', '') or '10.214.45.45'
    southfield_host = env.get('southfield_host', '') or env.get('SOUTHFIELD_HOST', '') or '10.192.224.131'

    if not netid or not password or not (krakow_path or southfield_path):
        print('ERROR: .env must set netid, netid_password, and krakow_path and/or southfield_path')
        raise SystemExit(1)

    targets = []
    if krakow_path:
        targets.append(('krakow', krakow_host if southfield_path else (host or krakow_host), krakow_path))
    if southfield_path:
        targets.append(('southfield', southfield_host if krakow_path else (host or southfield_host), southfield_path))

    # Runtime data dirs that must NOT be overwritten (exist cluster-side with real data)
    _RUNTIME_EXCLUDE_DIRS = {'store/db', 'store/logs', 'store/rag/vector_store', 'store'}

    import paramiko
    meta = _read_meta()
    stored = {_rel_key(k): v for k, v in meta.get('upload', {}).items()}
    new_hashes = {}
    changed = 0
    skipped = 0
    excluded = 0

    files_to_upload = []
    eligible_files = []
    for f in sorted(GEN.rglob('*')):
        if f.is_dir() or f.name.startswith('.'):
            continue
        rel = _rel_key(f.relative_to(GEN))
        # Skip runtime data dirs (cluster has real data that must persist)
        if any(rel.startswith(ex + '/') or rel == ex for ex in _RUNTIME_EXCLUDE_DIRS):
            excluded += 1
            continue
        if _is_model_binary(f):
            new_hashes[rel] = '__name_only__'
            eligible_files.append((rel, f, '__name_only__'))
            if rel in stored:
                skipped += 1
                continue
            files_to_upload.append((rel, f, '__name_only__'))
            continue
        sha = _sha256(f)
        new_hashes[rel] = sha
        eligible_files.append((rel, f, sha))
        if rel in stored and stored[rel] == sha:
            skipped += 1
            continue
        files_to_upload.append((rel, f, sha))

    if not files_to_upload and meta.get('upload_state') != 'remote':
        print('Upload metadata is not confirmed from a successful remote sync; forcing a full upload once.')
        files_to_upload = eligible_files
        skipped = 0

    total = len(files_to_upload)
    if total == 0:
        print(f'All files unchanged ({skipped} skipped, {excluded} runtime dirs excluded)')
        meta['upload'] = new_hashes
        _save_meta(meta)
        return

    print(f'Uploading {total} changed files ({skipped} unchanged, {excluded} runtime dirs excluded)...')

    failures = []
    failures_lock = threading.Lock()
    changed_lock = threading.Lock()

    def _progress_callback(target_name, index, total_files, local_path, remote_path, size, started):
        last_logged_at = started
        last_logged_bytes = 0

        def _callback(sent, total_bytes):
            nonlocal last_logged_at, last_logged_bytes
            now = time.perf_counter()
            if sent == total_bytes or sent == 0:
                should_log = True
            else:
                should_log = (now - last_logged_at >= 10.0) or (sent - last_logged_bytes >= 32 * 1024 * 1024)
            if not should_log:
                return
            elapsed = max(now - started, 0.001)
            pct = (sent / total_bytes * 100.0) if total_bytes else 100.0
            rate_mib = (sent / elapsed) / (1024 * 1024)
            print(
                f'[{target_name}] [{index}/{total_files}] PROGRESS '
                f'local={local_path} remote={remote_path} '
                f'sent={sent}/{total_bytes or size}B pct={pct:.1f}% '
                f'elapsed={elapsed:.2f}s rate={rate_mib:.2f}MiB/s',
                flush=True,
            )
            last_logged_at = now
            last_logged_bytes = sent

        return _callback

    def _connect_target(target_name, target_host, remote_root):
        print(f'[{target_name}] Upload target: {netid}@{target_host}:{port}')
        print(f'[{target_name}] Remote root: {remote_root}')
        print(f'[{target_name}] Connecting via SFTP...', end=' ', flush=True)
        sock = socket.create_connection((target_host, port), timeout=timeout_s)
        transport = paramiko.Transport(sock)
        transport.connect(username=netid, password=password)
        transport.set_keepalive(30)
        sftp = paramiko.SFTPClient.from_transport(
            transport,
            window_size=64 * 1024 * 1024,
            max_packet_size=4 * 1024 * 1024,
        )
        sftp.get_channel().settimeout(timeout_s)
        ensured_dirs = {'/'}
        _sftp_ensure_dir(sftp, remote_root, ensured_dirs)
        print('connected')
        return transport, sftp, ensured_dirs

    def _upload_target(target_name, target_host, remote_root):
        nonlocal changed
        transport = None
        sftp = None
        ensured_dirs = None
        try:
            transport, sftp, ensured_dirs = _connect_target(target_name, target_host, remote_root)
            for index, (rel, local_path, sha) in enumerate(files_to_upload, start=1):
                remote_path = str(PurePosixPath(remote_root) / rel)
                remote_dir = str(PurePosixPath(remote_path).parent)
                size = local_path.stat().st_size
                started = time.perf_counter()
                stage = 'ensure_dir'
                print(
                    f'[{target_name}] [{index}/{total}] START '
                    f'local={local_path} remote={remote_path} size={size}B',
                    flush=True,
                )
                try:
                    _sftp_ensure_dir(sftp, remote_dir, ensured_dirs)
                    stage = 'put'
                    sftp.put(
                        str(local_path),
                        remote_path,
                        callback=_progress_callback(target_name, index, total, local_path, remote_path, size, started),
                        confirm=True,
                    )
                    if rel.endswith('.sh') or local_path.suffix.lower() == '.sh':
                        stage = 'chmod'
                        sftp.chmod(remote_path, 0o755)
                    stage = 'verify'
                    remote_attr = sftp.stat(remote_path)
                    elapsed = time.perf_counter() - started
                    with changed_lock:
                        changed += 1
                    print(
                        f'[{target_name}] [{index}/{total}] DONE '
                        f'local={local_path} remote={remote_path} '
                        f'size={size}B remote_size={getattr(remote_attr, "st_size", "?")}B '
                        f'elapsed={elapsed:.2f}s',
                        flush=True,
                    )
                except Exception as exc:
                    elapsed = time.perf_counter() - started
                    msg = (
                        f'[{target_name}] [{index}/{total}] FAIL stage={stage} '
                        f'local={local_path} remote={remote_path} size={size}B '
                        f'elapsed={elapsed:.2f}s error={exc}'
                    )
                    print(msg, flush=True)
                    with failures_lock:
                        failures.append(msg)
        except Exception as exc:
            msg = f'[{target_name}] FAIL stage=connect remote_root={remote_root} error={exc}'
            print(msg, flush=True)
            with failures_lock:
                failures.append(msg)
        finally:
            if sftp is not None:
                sftp.close()
            if transport is not None:
                transport.close()

    target_threads = []
    for target_name, target_host, remote_root in targets:
        target_thread = threading.Thread(
            target=_upload_target,
            args=(target_name, target_host, remote_root),
            daemon=True,
        )
        target_thread.start()
        target_threads.append(target_thread)

    for target_thread in target_threads:
        target_thread.join()

    if failures:
        print(f'ERROR: {len(failures)} uploads failed:')
        for f in failures:
            print(f'  {f}')
    else:
        meta['upload'] = new_hashes
        meta['upload_state'] = 'remote'
        _save_meta(meta)

    print(f'Upload complete: {changed} uploaded, {skipped} unchanged, {excluded} runtime dirs excluded')
    if failures:
        raise SystemExit(1)


def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else 'deploy'
    if mode == 'generate':
        print('=== Generating deployment folder ===')
        generate()
    elif mode == 'upload':
        print('=== Uploading to cluster ===')
        upload()
    elif mode == 'deploy':
        print('=== Deploy: generate + upload ===')
        if platform.system() == 'Windows':
            print('  Running generate in WSL (apptainer)...')
            _run_in_wsl(['generate'])
            print('  Running upload from Windows (paramiko)...')
            upload()
        else:
            generate()
            upload()
    else:
        print(f'Usage: {sys.argv[0]} [generate|upload|deploy]')
        print('  default: deploy (build + assemble + upload)')
        sys.exit(1)


if __name__ == '__main__':
    main()
