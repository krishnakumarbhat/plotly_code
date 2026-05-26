"""Get full launcher script content to see singularity exec command."""
from __future__ import annotations
import argparse, sys, io, paramiko

PROBE = '''
import zipfile, os, subprocess, glob

PYZ = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz"

with zipfile.ZipFile(PYZ) as z:
    content = z.read("hpcc_main.py").decode("utf-8", errors="replace")
lines = content.splitlines()

# Find _tool_command and the singularity exec line
print("=== _tool_command function (up to 80 lines) ===")
in_func = False
count = 0
for i, line in enumerate(lines):
    if "def _tool_command" in line:
        in_func = True
    if in_func:
        print(str(i+1)+": "+line)
        count += 1
        if count > 80:
            break
        if count > 5 and line.strip().startswith("def "):
            break

print()
print("=== Full launcher script (most recent pcmzxl run) ===")
runs_base = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/runs"
launchers = sorted(glob.glob(runs_base + "/pcmzxl/*/slurm_tmux_launcher.sh"), key=os.path.getmtime, reverse=True)
if launchers:
    f = launchers[0]
    print("File:", f)
    try:
        with open(f) as fh:
            print(fh.read())
    except Exception as e:
        print("ERROR:", e)

print()
print("=== Check group 1579655975 ===")
import grp
try:
    g = grp.getgrgid(1579655975)
    print("Group name:", g.gr_name)
except Exception as e:
    print("getgrgid error:", e)

print()
print("=== pcmzxl full groups ===")
r = subprocess.run(["id", "pcmzxl"], capture_output=True, text=True)
print(r.stdout.strip())

print()
print("=== Is pcmzxl in group 1579655975? ===")
import grp as grpmod
try:
    g = grpmod.getgrgid(1579655975)
    print("Group members:", g.gr_mem[:20])
    print("pcmzxl in it:", "pcmzxl" in g.gr_mem)
except Exception as e:
    print("error:", e)

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
        sftp.putfo(io.BytesIO(PROBE.encode()), "/tmp/_probe5.py")
        sftp.close()
        _, o, e = c.exec_command("python3 /tmp/_probe5.py", timeout=30)
        sys.stdout.write(o.read().decode("utf-8", errors="replace"))
        err = e.read().decode("utf-8", errors="replace")
        if err.strip():
            sys.stderr.write(err)
    finally:
        c.close()

if __name__ == "__main__":
    main()
