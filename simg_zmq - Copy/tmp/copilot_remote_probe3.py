"""Probe srun command construction and test --uid capability."""
from __future__ import annotations
import argparse, sys, io, paramiko

PROBE = '''
import zipfile, os, subprocess

PYZ = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz"

with zipfile.ZipFile(PYZ) as z:
    content = z.read("hpcc_main.py").decode("utf-8", errors="replace")
lines = content.splitlines()

print("=== srun_command construction (lines 1320-1380) ===")
for i in range(1319, min(1380, len(lines))):
    print(str(i+1)+": "+lines[i])

print()
print("=== Look for 'uid' or 'user' in srun context (lines 1300-1550) ===")
for i in range(1299, min(1550, len(lines))):
    ll = lines[i].lower()
    if "uid" in ll or ("user" in ll and "srun" in lines[max(0,i-10):i+1].__repr__()):
        print(str(i+1)+": "+lines[i])

print()
print("=== Test: can ouymc2 use srun --uid? ===")
srun = "/usr/bin/srun"
if not os.path.exists(srun):
    import shutil
    srun = shutil.which("srun") or "srun"
r = subprocess.run([srun, "--uid=pcmzxl", "--test-only", "whoami"],
                   capture_output=True, text=True, timeout=10)
print("srun --uid test stdout:", r.stdout.strip())
print("srun --uid test stderr:", r.stderr.strip()[:300])
print("returncode:", r.returncode)

print()
print("=== Check pcmzxl user exists ===")
import pwd
try:
    u = pwd.getpwnam("pcmzxl")
    print("pcmzxl uid:", u.pw_uid, "gid:", u.pw_gid)
except Exception as e:
    print("pcmzxl lookup:", e)

print("DONE")
'''

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--password", required=True)
    p.add_argument("--host", default="10.192.224.131")
    p.add_argument("--user", default="ouymc2")
    args = p.parse_args()
    c = paramiko.SSHClient()
    c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    c.connect(args.host, username=args.user, password=args.password,
              look_for_keys=False, allow_agent=False, timeout=20)
    try:
        sftp = c.open_sftp()
        sftp.putfo(io.BytesIO(PROBE.encode()), "/tmp/_probe3.py")
        sftp.close()
        _, o, e = c.exec_command("python3 /tmp/_probe3.py", timeout=30)
        sys.stdout.write(o.read().decode("utf-8", errors="replace"))
        err = e.read().decode("utf-8", errors="replace")
        if err.strip():
            sys.stderr.write(err)
    finally:
        c.close()

if __name__ == "__main__":
    main()
