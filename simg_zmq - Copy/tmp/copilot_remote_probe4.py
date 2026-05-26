"""Check singularity command in job launcher and test mkdir as pcmzxl."""
from __future__ import annotations
import argparse, sys, io, paramiko

PROBE = '''
import zipfile, os, subprocess, glob

PYZ = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz"

with zipfile.ZipFile(PYZ) as z:
    content = z.read("hpcc_main.py").decode("utf-8", errors="replace")
lines = content.splitlines()

print("=== Singularity command lines in hpcc_main.py ===")
for i, line in enumerate(lines):
    ll = line.lower()
    if "singularity" in ll or "userns" in ll or "no-home" in ll or "fakeroot" in ll:
        print(str(i+1)+": "+line)

print()
print("=== Recent slurm_tmux_launcher.sh files ===")
runs_base = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/runs"
launchers = sorted(glob.glob(runs_base + "/*/*/*launcher*.sh"), key=os.path.getmtime, reverse=True)[:3]
for f in launchers:
    print("\\n--- " + f + " ---")
    try:
        with open(f) as fh:
            txt = fh.read()
        for line in txt.splitlines():
            ll = line.lower()
            if "singularity" in ll or "srun" in ll or "mkdir" in ll or "output" in ll or "uid" in ll:
                print("  " + line[:200])
    except Exception as e:
        print("  ERROR: " + str(e))

print()
print("=== Test: can sudo -u pcmzxl mkdir? ===")
r = subprocess.run(["sudo", "-n", "-u", "pcmzxl", "id"], capture_output=True, text=True, timeout=10)
print("sudo -u pcmzxl id stdout:", r.stdout.strip())
print("sudo -u pcmzxl id stderr:", r.stderr.strip()[:200])
print("returncode:", r.returncode)

print()
print("=== Check GPO-IFV7XX project root perms ===")
path = "/mnt/usmidet/projects/GPO-IFV7XX"
try:
    st = os.stat(path)
    import stat as statmod
    print("GPO-IFV7XX mode:", oct(st.st_mode), "uid:", st.st_uid, "gid:", st.st_gid)
    # Check if GPO-IFV7XX/8-Users exists
    users_path = path + "/8-Users"
    if os.path.exists(users_path):
        st2 = os.stat(users_path)
        print("8-Users mode:", oct(st2.st_mode), "uid:", st2.st_uid, "gid:", st2.st_gid)
    else:
        print("8-Users: Permission denied or not exists")
except Exception as e:
    print("stat error:", e)

print()
print("=== Groups of ouymc2 and pcmzxl ===")
import pwd, grp
try:
    ouymc2_gids = os.getgroups()
    ouymc2_groups = []
    for gid in ouymc2_gids:
        try:
            ouymc2_groups.append(grp.getgrgid(gid).gr_name)
        except:
            ouymc2_groups.append(str(gid))
    print("ouymc2 groups:", ouymc2_groups[:15])
except Exception as e:
    print("groups error:", e)

# Check what groups pcmzxl belongs to
try:
    pcmzxl_uid = pwd.getpwnam("pcmzxl").pw_uid
    r = subprocess.run(["id", "pcmzxl"], capture_output=True, text=True)
    print("pcmzxl id:", r.stdout.strip()[:300])
except Exception as e:
    print("pcmzxl groups error:", e)

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
        sftp.putfo(io.BytesIO(PROBE.encode()), "/tmp/_probe4.py")
        sftp.close()
        _, o, e = c.exec_command("python3 /tmp/_probe4.py", timeout=30)
        sys.stdout.write(o.read().decode("utf-8", errors="replace"))
        err = e.read().decode("utf-8", errors="replace")
        if err.strip():
            sys.stderr.write(err)
    finally:
        c.close()

if __name__ == "__main__":
    main()
