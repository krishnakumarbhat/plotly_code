"""Test sbatch --uid and probe Krakow - Python 3.6 compatible."""
from __future__ import annotations
import argparse, sys, io, paramiko

PROBE_SOUTH = '''
import subprocess

print("=== Test sbatch --uid ===")
r = subprocess.Popen(
    ["sbatch", "--uid=pcmzxl", "--test-only", "--wrap=whoami"],
    stdout=subprocess.PIPE, stderr=subprocess.PIPE
)
out, err = r.communicate(timeout=10)
print("sbatch --uid stdout:", out.decode("utf-8", errors="replace").strip())
print("sbatch --uid stderr:", err.decode("utf-8", errors="replace").strip()[:300])
print("returncode:", r.returncode)

print()
print("=== Test su -c mkdir as pcmzxl ===")
r2 = subprocess.Popen(
    ["su", "-", "pcmzxl", "-c", "mkdir -p /tmp/test_pcmzxl_copilot"],
    stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE
)
out2, err2 = r2.communicate(timeout=5)
print("su stdout:", out2.decode("utf-8", errors="replace").strip())
print("su stderr:", err2.decode("utf-8", errors="replace").strip()[:200])
print("returncode:", r2.returncode)

print()
print("=== Check GPO-IFV7XX/2-Sim/USER_DATA (accessible?) ===")
import os
for p in [
    "/mnt/usmidet/projects/GPO-IFV7XX/4-Checkout",
    "/mnt/usmidet/projects/GPO-IFV7XX/2-Sim",
]:
    try:
        st = os.stat(p)
        print(p, "mode:", oct(st.st_mode), "gid:", st.st_gid, "write:", os.access(p, os.W_OK))
    except Exception as e:
        print(p, "ERROR:", e)

print("DONE")
'''

PROBE_KRAKOW = '''
import zipfile
import os
import subprocess

print("=== Krakow whoami ===")
p = subprocess.Popen(["whoami"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, err = p.communicate()
print("whoami:", out.decode("utf-8", errors="replace").strip())

p2 = subprocess.Popen(["id"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out2, err2 = p2.communicate()
print("id:", out2.decode("utf-8", errors="replace").strip()[:300])

print()
print("=== Krakow _resolve_output_path ===")
for pyz_path in [
    "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/hpcc_main.pyz",
    "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/hpcc_main.pyz",
]:
    if not os.path.exists(pyz_path):
        print("NOT FOUND:", pyz_path)
        continue
    print("Found:", pyz_path)
    try:
        with zipfile.ZipFile(pyz_path) as z:
            content = z.read("hpcc_main.py").decode("utf-8", errors="replace")
        lines = content.splitlines()
        for i, line in enumerate(lines):
            if "def _resolve_output_path" in line:
                for j in range(i, min(i+35, len(lines))):
                    print(str(j+1)+": "+lines[j])
                    if j > i and lines[j].strip().startswith("def "):
                        break
                break
    except Exception as e:
        print("Error:", e)
    break

print()
print("=== Recent Krakow run launchers (srun line) ===")
import glob
for base in [
    "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs",
    "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/runs",
]:
    if not os.path.exists(base):
        continue
    launchers = sorted(
        glob.glob(base + "/*/*/*launcher*.sh"),
        key=os.path.getmtime, reverse=True
    )[:2]
    for f in launchers:
        print("--- " + f + " ---")
        try:
            with open(f) as fh:
                for line in fh.read().splitlines():
                    if "srun" in line or "singularity" in line or "uid" in line.lower() or "output" in line:
                        print("  " + line[:200])
        except Exception as e:
            print("  ERROR:", e)

print("DONE")
'''


def probe(host, user, password, script, label):
    c = paramiko.SSHClient()
    c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    c.connect(host, username=user, password=password,
              look_for_keys=False, allow_agent=False, timeout=20)
    try:
        sftp = c.open_sftp()
        sftp.putfo(io.BytesIO(script.encode()), "/tmp/_probe7.py")
        sftp.close()
        _, o, e = c.exec_command("python3 /tmp/_probe7.py 2>&1", timeout=30)
        out = o.read().decode("utf-8", errors="replace")
        print("\n" + "="*60)
        print(label)
        print("="*60)
        print(out)
    finally:
        c.close()


p = argparse.ArgumentParser()
p.add_argument("--password", required=True)
args = p.parse_args()

probe("10.192.224.131", "ouymc2", args.password, PROBE_SOUTH, "SOUTHFIELD - sbatch/su tests")
try:
    probe("10.214.45.45", "ouymc2", args.password, PROBE_KRAKOW, "KRAKOW - code comparison")
except Exception as e:
    print("\nKRAKOW connection failed:", e)
