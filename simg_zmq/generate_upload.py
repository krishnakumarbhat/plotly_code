import os, platform, shutil, subprocess, sys, json, hashlib
from pathlib import Path

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

SCRIPTS = ['bundle_common.sh', 'cleanup_memory.sh', 'hpcc_runtime.env', 'kpi_runtime_launcher.sh']
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
            check=True, env=build_env,
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
        shutil.copytree(ROOT / 'rag', dst, ignore=shutil.ignore_patterns('__pycache__', '*.pyc', 'model', 'embding_mod', 'tools', 'data'))
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
    _bundle_model_dir(ROOT / 'rag' / 'embding_mod', GEN / 'rag' / 'embding_mod')

    for sub in ['db', 'logs', 'rag/vector_store']:
        (GEN / 'store' / sub).mkdir(parents=True, exist_ok=True)
    print('  created store/')

    _write_launcher()
    _fix_shell_scripts()
    _ensure_rag_sh()
    _write_readme()
    _save_upload_hashes()
    print(f'Generate complete: {GEN}')


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
        rel = str(f.relative_to(GEN))
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


def _sftp_ensure_dir(sftp, path):
    parts = path.strip('/').split('/')
    for i in range(1, len(parts) + 1):
        p = '/' + '/'.join(parts[:i])
        try:
            sftp.stat(p)
        except FileNotFoundError:
            sftp.mkdir(p)


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
    host = env.get('host', '')
    port = int(env.get('port', '22'))

    if not host:
        if krakow_path:
            host = '10.214.45.45'
        elif southfield_path:
            host = '10.192.224.131'
    if not netid or not password or not host:
        print('ERROR: .env must set netid, netid_password, and host (or krakow_path/southfield_path)')
        raise SystemExit(1)

    remote_root = krakow_path or southfield_path
    print(f'Upload target: {netid}@{host}:{port}')
    print(f'Remote root: {remote_root}')

    # Runtime data dirs that must NOT be overwritten (exist cluster-side with real data)
    _RUNTIME_EXCLUDE_DIRS = {'store/db', 'store/logs', 'store/rag/vector_store', 'store'}

    import paramiko
    meta = _read_meta()
    stored = meta.get('upload', {})
    new_hashes = {}
    changed = 0
    skipped = 0
    excluded = 0

    print('Connecting via SFTP...', end=' ', flush=True)
    transport = paramiko.Transport((host, port))
    transport.connect(username=netid, password=password)
    sftp = paramiko.SFTPClient.from_transport(transport)
    _sftp_ensure_dir(sftp, remote_root)
    print('connected')

    files_to_upload = []
    for f in sorted(GEN.rglob('*')):
        if f.is_dir() or f.name.startswith('.'):
            continue
        rel = str(f.relative_to(GEN))
        # Skip runtime data dirs (cluster has real data that must persist)
        if any(rel.startswith(ex + '/') or rel == ex for ex in _RUNTIME_EXCLUDE_DIRS):
            excluded += 1
            continue
        if _is_model_binary(f):
            if rel in stored:
                skipped += 1
                continue
            files_to_upload.append((rel, f, '__name_only__'))
            continue
        sha = _sha256(f)
        new_hashes[rel] = sha
        if rel in stored and stored[rel] == sha:
            skipped += 1
            continue
        files_to_upload.append((rel, f, sha))

    total = len(files_to_upload)
    if total == 0:
        print(f'All files unchanged ({skipped} skipped, {excluded} runtime dirs excluded)')
    else:
        print(f'Uploading {total} changed files ({skipped} unchanged, {excluded} runtime dirs excluded)...')

    import threading

    def _upload_file(rel, local_path):
        remote = f'{remote_root}/{rel}'
        try:
            _sftp_ensure_dir(sftp, str(Path(remote).parent))
            sftp.put(str(local_path), remote)
            return rel, True, None
        except Exception as e:
            return rel, False, str(e)

    MAX_WORKERS = 4
    results = []
    sem = threading.Semaphore(MAX_WORKERS)

    def _worker(rel, local_path):
        sem.acquire()
        try:
            r = _upload_file(rel, local_path)
        finally:
            sem.release()
        results.append(r)

    threads = []
    for rel, f, sha in files_to_upload:
        t = threading.Thread(target=_worker, args=(rel, f), daemon=True)
        t.start()
        threads.append(t)

    for t in threads:
        t.join()

    successes = 0
    failures = []
    for rel, ok, err in results:
        if ok:
            successes += 1
            print(f'  OK: {rel}')
            changed += 1
        else:
            failures.append(f'{rel}: {err}')
            print(f'  FAIL: {rel}: {err}')

    sftp.close()
    transport.close()

    if failures:
        print(f'ERROR: {len(failures)} uploads failed:')
        for f in failures:
            print(f'  {f}')
    else:
        new_hashes.update({k: v for k, v in stored.items() if v == '__name_only__'})
        meta['upload'] = new_hashes
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
