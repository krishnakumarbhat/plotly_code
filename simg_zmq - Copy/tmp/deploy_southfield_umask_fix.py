"""Deploy umask+reuse-check+WAL fixes to Southfield, then restart full stack."""
from __future__ import annotations
import posixpath, stat, time
from pathlib import Path
import paramiko

HOST = "10.192.224.131"
USER = "ouymc2"
PASSWORD = "Zalikapope@202425"
REMOTE_ROOT = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service"
LOCAL_BUNDLE = Path(r"C:\Users\ouymc2\Desktop\simg_zmq\simg_sh_hpcc")

RESTART_SCRIPT = f"""#!/usr/bin/env bash
set -euo pipefail
cd {REMOTE_ROOT}
mkdir -p logs

# Kill all services
for pattern in 'run_hpcc_stack.sh' 'main_hpcc.sh' 'hpcc_main.pyz' 'main_html.simg' 'gunicorn'; do
  pkill -u {USER} -f "$pattern" >/dev/null 2>&1 || true
done
sleep 2
for pattern in 'hpcc_main.pyz' 'main_html.simg'; do
  pkill -9 -u {USER} -f "$pattern" >/dev/null 2>&1 || true
done
sleep 1

chmod +x hpcc_main.pyz bundle_common.sh main_hpcc.sh run_hpcc_stack.sh kpi_runtime_launcher.sh 2>/dev/null || true

logfile=logs/restart_$(date +%Y%m%d_%H%M%S).log
nohup env HPCC_PUBLIC_HOST=10.192.224.131 PORT=5002 HPCC_BROKER_PORT=9100 HPCC_PORT_CONFLICT_POLICY=fail HPCC_AUTO_START_RAG=1 ./run_hpcc_stack.sh > "$logfile" 2>&1 < /dev/null &
echo "PID=$!"
sleep 3

# Verify umask fix is in new pyz
python3 -c "
import zipfile
with zipfile.ZipFile('{REMOTE_ROOT}/hpcc_main.pyz') as z:
    src = z.read('hpcc_main.py').decode()
    for check, label in [('umask(0o022)', 'umask-022'), ('ssh_run_as_user', 'ssh-as-user')]:
        print(label + ': ' + ('OK' if check in src else 'MISSING'))
"

for _ in $(seq 1 45); do
  if ss -ltn | grep -q ':9100 ' && ss -ltn | grep -q ':5002 '; then
    break
  fi
  sleep 2
done
echo '=== PORTS ==='
ss -ltnp | grep -E ':5002|:9100' || true
echo '=== BROKER PING ==='
python3 -c "
import json, socket
conn = socket.create_connection(('127.0.0.1', 9100), timeout=5)
conn.sendall(json.dumps({{'action': 'ping'}}).encode() + b'\\n')
buf = b''
while not buf.endswith(b'\\n'):
    chunk = conn.recv(4096)
    if not chunk: break
    buf += chunk
conn.close()
r = json.loads(buf.decode().strip())
print('broker ok:', r.get('ok'), 'status:', r.get('status'))
" 2>&1 || echo 'broker not ready yet'
echo '=== RESTART LOG (last 40) ==='
tail -n 40 "$logfile" 2>/dev/null || true
"""


def upload_file(sftp: paramiko.SFTPClient, local: Path, remote: str) -> None:
    sftp.put(str(local), remote)
    if local.suffix in {'.sh', '.pyz'} or (local.stat().st_mode & stat.S_IXUSR):
        sftp.chmod(remote, 0o775)
    else:
        sftp.chmod(remote, 0o644)


def main() -> None:
    stamp = time.strftime('%Y%m%d_%H%M%S')
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(HOST, username=USER, password=PASSWORD,
                   look_for_keys=False, allow_agent=False, timeout=20)
    print(f"Connected to {HOST}")
    try:
        sftp = client.open_sftp()
        try:
            # 1. Backup and upload new pyz
            pyz_local = LOCAL_BUNDLE / "hpcc_main.pyz"
            pyz_remote = posixpath.join(REMOTE_ROOT, "hpcc_main.pyz")
            bak = posixpath.join(REMOTE_ROOT, f"hpcc_main.pyz.bak_umask_{stamp}")
            try:
                sftp.stat(pyz_remote)
                _, o, _ = client.exec_command(f"cp {pyz_remote} {bak}", timeout=10)
                o.read()
                print(f"Backed up pyz to {bak}")
            except OSError:
                pass
            upload_file(sftp, pyz_local, pyz_remote)
            info = sftp.stat(pyz_remote)
            print(f"Uploaded pyz: {info.st_size} bytes")

            # 2. Upload updated main_html files (app.py, runtime_store.py)
            main_html_remote = posixpath.join(REMOTE_ROOT, "bundle_src", "main_html")
            for fname in ["app.py", "runtime_store.py", "hpcc_broker_client.py",
                          "env_utils.py", "config.py", "models.py", "utils.py"]:
                local_path = LOCAL_BUNDLE / "bundle_src" / "main_html" / fname
                if local_path.exists():
                    remote_path = posixpath.join(main_html_remote, fname)
                    upload_file(sftp, local_path, remote_path)
                    print(f"Uploaded {fname}")

            # 3. Upload restart script
            rs_path = posixpath.join(REMOTE_ROOT, ".copilot_restart_umask.sh")
            with sftp.open(rs_path, 'w') as fh:
                fh.write(RESTART_SCRIPT)
            sftp.chmod(rs_path, 0o755)
        finally:
            sftp.close()

        # 4. Run restart
        print("\n=== Running restart script ===")
        _, stdout, stderr = client.exec_command(f"bash {rs_path}", timeout=300)
        out = stdout.read().decode('utf-8', errors='replace')
        err = stderr.read().decode('utf-8', errors='replace')
        print(out)
        if err.strip():
            print("STDERR:", err[:500])
    finally:
        client.close()
    print("\nDone.")


if __name__ == "__main__":
    main()
