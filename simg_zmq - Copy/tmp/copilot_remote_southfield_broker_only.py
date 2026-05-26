from __future__ import annotations

import posixpath
import stat
from pathlib import Path

import paramiko

HOST = "10.192.224.131"
USER = "ouymc2"
PASSWORD = "Zalikapope@202425"
REMOTE_ROOT = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service"
LOCAL_FILE = Path(r"C:\Users\ouymc2\Desktop\simg_zmq\simg_sh_hpcc\hpcc_main.pyz")
REMOTE_FILE = posixpath.join(REMOTE_ROOT, "hpcc_main.pyz")
RESTART_SCRIPT = f"""#!/usr/bin/env bash
set -euo pipefail
cd {REMOTE_ROOT}
for pattern in 'run_hpcc_stack.sh' 'main_hpcc.sh' 'hpcc_main.pyz' 'main_html.simg' 'run_rag.sh' 'rag.simg' 'gunicorn'; do
  pkill -u {USER} -f "$pattern" >/dev/null 2>&1 || true
  pkill -9 -u {USER} -f "$pattern" >/dev/null 2>&1 || true
done
logfile=logs/restart_$(date +%Y%m%d_%H%M%S).log
nohup env HPCC_PUBLIC_HOST=10.192.224.131 PORT=5002 HPCC_BROKER_PORT=9100 HPCC_PORT_CONFLICT_POLICY=fail HPCC_AUTO_START_RAG=1 RAG_PORT=5100 ./run_hpcc_stack.sh > "$logfile" 2>&1 < /dev/null &
for _ in $(seq 1 45); do
  if ss -ltn | grep -q ':9100 ' && ss -ltn | grep -q ':5002 '; then
    break
  fi
  sleep 2
done
echo PORTS
ss -ltnp | grep -E ':5002|:9100|:5100' || true
echo '--- restart log ---'
tail -n 80 "$logfile" || true
"""


def ensure_remote_dir(sftp: paramiko.SFTPClient, remote_dir: str) -> None:
    normalized = posixpath.normpath(remote_dir)
    pending = []
    while normalized not in {"/", "."}:
        pending.append(normalized)
        normalized = posixpath.dirname(normalized)
    for directory in reversed(pending):
        try:
            sftp.stat(directory)
        except OSError:
            sftp.mkdir(directory)


def main() -> int:
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(HOST, username=USER, password=PASSWORD, look_for_keys=False, allow_agent=False, timeout=20)
    try:
        sftp = client.open_sftp()
        try:
            ensure_remote_dir(sftp, posixpath.dirname(REMOTE_FILE))
            sftp.put(str(LOCAL_FILE), REMOTE_FILE)
            sftp.chmod(REMOTE_FILE, 0o775)
            remote_script = posixpath.join(REMOTE_ROOT, '.copilot_restart_broker_only.sh')
            with sftp.open(remote_script, 'w') as handle:
                handle.write(RESTART_SCRIPT)
            sftp.chmod(remote_script, 0o775)
        finally:
            sftp.close()

        stdin, stdout, stderr = client.exec_command(f"bash {posixpath.join(REMOTE_ROOT, '.copilot_restart_broker_only.sh')}", timeout=420)
        print(stdout.read().decode('utf-8', errors='replace'), end='')
        err = stderr.read().decode('utf-8', errors='replace')
        if err.strip():
            print('\n--- STDERR ---')
            print(err, end='')
    finally:
        client.close()
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
