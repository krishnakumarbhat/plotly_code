"""Upload and run a detailed probe via SFTP+SSH."""
from __future__ import annotations
import argparse
import sys
import io
import paramiko

PROBE = '''
import zipfile, os

PYZ = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz"

with zipfile.ZipFile(PYZ) as z:
    content = z.read("hpcc_main.py").decode("utf-8", errors="replace")

lines = content.splitlines()

print("=== Slurm submission lines (srun/sbatch/uid) ===")
for i, line in enumerate(lines):
    ll = line.lower()
    if "srun" in ll or "sbatch" in ll or "--uid" in ll:
        print(str(i+1) + ": " + line)

print()
print("=== _resolve_output_path function ===")
in_func = False
for i, line in enumerate(lines):
    if "def _resolve_output_path" in line:
        in_func = True
    if in_func:
        print(str(i+1) + ": " + line)
        if i > 0 and line.strip().startswith("def ") and "resolve" not in line:
            break
        if in_func and i > lines.index(lines[i]) + 40:
            break

print()
print("=== Who is running as (check user) ===")
import subprocess
r = subprocess.run(["whoami"], capture_output=True, text=True)
print("whoami:", r.stdout.strip())
r2 = subprocess.run(["id"], capture_output=True, text=True)
print("id:", r2.stdout.strip())

print()
print("=== Check pcmzxl path writability as ouymc2 ===")
import os
test_path = "/mnt/usmidet/projects/GPO-IFV7XX/8-Users/pcmzxl"
print("path exists:", os.path.exists(test_path))
print("access R_OK:", os.access(test_path, os.R_OK))
print("access W_OK:", os.access(test_path, os.W_OK))
try:
    st = os.stat(test_path)
    print("stat:", oct(st.st_mode), "uid:", st.st_uid, "gid:", st.st_gid)
except Exception as e:
    print("stat error:", e)

import grp, pwd
try:
    print("ouymc2 uid:", pwd.getpwnam("ouymc2").pw_uid)
except:
    pass

print("DONE")
'''

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--password", required=True)
    parser.add_argument("--host", default="10.192.224.131")
    parser.add_argument("--user", default="ouymc2")
    args = parser.parse_args()

    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(args.host, username=args.user, password=args.password,
                   look_for_keys=False, allow_agent=False, timeout=20)
    try:
        sftp = client.open_sftp()
        sftp.putfo(io.BytesIO(PROBE.encode()), "/tmp/_copilot_probe2.py")
        sftp.close()
        _, stdout, stderr = client.exec_command("python3 /tmp/_copilot_probe2.py", timeout=30)
        sys.stdout.write(stdout.read().decode("utf-8", errors="replace"))
        err = stderr.read().decode("utf-8", errors="replace")
        if err.strip():
            sys.stderr.write(err)
    finally:
        client.close()
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
