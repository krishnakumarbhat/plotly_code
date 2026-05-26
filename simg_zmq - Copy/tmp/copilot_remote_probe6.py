"""Probe Krakow cluster and get full pcmzxl group list."""
from __future__ import annotations
import argparse, sys, io, paramiko

PROBE_SOUTH = '''
import subprocess, os, grp

print("=== pcmzxl FULL groups ===")
r = subprocess.run(["id", "pcmzxl"], capture_output=True, text=True)
print(r.stdout.strip())
print(r.stderr.strip())

# Check if pcmzxl is in gpo-ifv7xx group
try:
    g = grp.getgrgid(1579655975)
    print("GPO-IFV7XX group name:", g.gr_name)
    print("pcmzxl in GPO-IFV7XX group:", "pcmzxl" in g.gr_mem)
    print("ouymc2 in GPO-IFV7XX group:", "ouymc2" in g.gr_mem)
except Exception as e:
    print("group lookup error:", e)

print()
print("=== How broker is started (run_hpcc_stack.sh) ===")
try:
    with open("/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/run_hpcc_stack.sh") as f:
        print(f.read()[:3000])
except Exception as e:
    print("ERROR:", e)

print()
print("=== Check if /mnt/usmidet/projects/GPO-IFV7XX paths accessible ===")
for p in [
    "/mnt/usmidet/projects/GPO-IFV7XX",
    "/mnt/usmidet/projects/GPO-IFV7XX/2-Sim",
    "/mnt/usmidet/projects/GPO-IFV7XX/2-Sim/USER_DATA",
    "/mnt/usmidet/projects/GPO-IFV7XX/8-Users",
]:
    try:
        st = os.stat(p)
        import stat as statmod
        mode = oct(st.st_mode)
        try:
            gname = grp.getgrgid(st.st_gid).gr_name
        except:
            gname = str(st.st_gid)
        w_ok = os.access(p, os.W_OK)
        print(f"{p}: mode={mode} gid={gname} writable_by_ouymc2={w_ok}")
    except Exception as e:
        print(f"{p}: {e}")

print("DONE")
'''

PROBE_KRAKOW = '''
import zipfile, os, subprocess, grp

print("=== Krakow - who am I ===")
r = subprocess.run(["whoami"], capture_output=True, text=True)
print("whoami:", r.stdout.strip())
r2 = subprocess.run(["id"], capture_output=True, text=True)
print("id:", r2.stdout.strip()[:400])

print()
print("=== Krakow all_service hpcc_main pyz - _resolve_output_path ===")
PYZ = "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/hpcc_main.pyz"
if not os.path.exists(PYZ):
    print("all_service pyz NOT FOUND:", PYZ)
    # Try all_service2
    PYZ = "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/hpcc_main.pyz"
    print("Trying all_service2:", PYZ, "exists:", os.path.exists(PYZ))

if os.path.exists(PYZ):
    with zipfile.ZipFile(PYZ) as z:
        content = z.read("hpcc_main.py").decode("utf-8", errors="replace")
    lines = content.splitlines()
    for i, line in enumerate(lines):
        if "def _resolve_output_path" in line:
            for j in range(i, min(i+35, len(lines))):
                print(str(j+1)+": "+lines[j])
                if j > i and lines[j].strip().startswith("def "):
                    break
            break

print()
print("=== Krakow all_service run_hpcc_stack.sh ===")
for f in [
    "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/run_hpcc_stack.sh",
    "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/run_hpcc_stack.sh",
]:
    if os.path.exists(f):
        print("File:", f)
        try:
            with open(f) as fh:
                print(fh.read()[:3000])
        except Exception as e:
            print("ERROR:", e)
        break

print()
print("=== Recent Krakow run launcher scripts ===")
import glob
for base in [
    "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/runs",
    "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/runs",
]:
    launchers = sorted(glob.glob(base + "/*/*/*launcher*.sh"), key=os.path.getmtime, reverse=True)[:2]
    for f in launchers:
        print("--- " + f + " ---")
        try:
            with open(f) as fh:
                for line in fh.read().splitlines():
                    if "srun" in line or "singularity" in line or "uid" in line.lower():
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
        sftp.putfo(io.BytesIO(script.encode()), "/tmp/_probe6.py")
        sftp.close()
        _, o, e = c.exec_command("python3 /tmp/_probe6.py", timeout=30)
        out = o.read().decode("utf-8", errors="replace")
        err = e.read().decode("utf-8", errors="replace")
        print(f"\n{'='*60}\n{label}\n{'='*60}")
        print(out)
        if err.strip():
            print("STDERR:", err[:500])
    finally:
        c.close()

import argparse
p = argparse.ArgumentParser()
p.add_argument("--password", required=True)
args = p.parse_args()

probe("10.192.224.131", "ouymc2", args.password, PROBE_SOUTH, "SOUTHFIELD")
try:
    probe("10.214.45.45", "ouymc2", args.password, PROBE_KRAKOW, "KRAKOW")
except Exception as e:
    print(f"\nKRAKOW connection failed: {e}")
