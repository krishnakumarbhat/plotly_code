"""Deploy fix to Krakow all_service2, then verify."""
from __future__ import annotations
import argparse, sys, io, paramiko

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--password", required=True)
    p.add_argument("--pyz", required=True)
    p.add_argument("--host", default="10.214.45.45")
    p.add_argument("--user", default="ouymc2")
    args = p.parse_args()

    remote_path = "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/hpcc_main.pyz"
    backup_path = "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/hpcc_main.pyz.bak_pre_fix"

    c = paramiko.SSHClient()
    c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    c.connect(args.host, username=args.user, password=args.password,
              look_for_keys=False, allow_agent=False, timeout=20)
    try:
        # Check if the file exists
        _, o, e = c.exec_command(f"ls -la {remote_path} 2>&1")
        print("File check:", o.read().decode().strip(), e.read().decode().strip()[:100])

        # Backup
        _, o2, e2 = c.exec_command(f"cp {remote_path} {backup_path} 2>&1 && echo 'backup ok' || echo 'backup failed'")
        print("Backup:", o2.read().decode().strip())

        # Upload
        sftp = c.open_sftp()
        sftp.put(args.pyz, remote_path)
        sftp.chmod(remote_path, 0o755)
        sftp.close()
        print("Upload: done")

        # Verify
        verify = """
import zipfile
with zipfile.ZipFile("/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/hpcc_main.pyz") as z:
    content = z.read("hpcc_main.py").decode()
    if "os.makedirs(candidate" in content:
        print("ERROR: old makedirs test still present!")
    else:
        print("OK: old makedirs test removed")
    if "mkdir_step" in content:
        print("OK: mkdir_step in launcher code")
    else:
        print("WARNING: mkdir_step not found")
    lines = content.splitlines()
    for i, line in enumerate(lines):
        if "def _resolve_output_path" in line:
            for j in range(i, min(i+8, len(lines))):
                print(str(j+1)+": "+lines[j])
            break
"""
        sftp2 = c.open_sftp()
        sftp2.putfo(io.BytesIO(verify.encode()), "/tmp/_verify_krakow.py")
        sftp2.close()
        _, o3, e3 = c.exec_command("python3 /tmp/_verify_krakow.py 2>&1", timeout=20)
        print(o3.read().decode("utf-8", errors="replace"))

        # Restart broker on Krakow
        restart = """
import subprocess, os, signal, time
r = subprocess.Popen(["pgrep", "-f", "hpcc_main.pyz"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, _ = r.communicate()
pids = [int(x.strip()) for x in out.decode().splitlines() if x.strip().isdigit()]
print("Krakow broker PIDs:", pids)
for pid in pids:
    try:
        os.kill(pid, signal.SIGTERM)
        print("Sent SIGTERM to", pid)
    except Exception as e:
        print("kill error:", e)
if pids:
    time.sleep(3)
    print("Broker stopped.")

# Restart via tmux if possible
r2 = subprocess.Popen(["tmux", "list-sessions"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out2, _ = r2.communicate()
print("Krakow tmux sessions:", out2.decode().strip()[:200])
print("DONE")
"""
        sftp3 = c.open_sftp()
        sftp3.putfo(io.BytesIO(restart.encode()), "/tmp/_restart_krakow.py")
        sftp3.close()
        _, o4, e4 = c.exec_command("python3 /tmp/_restart_krakow.py 2>&1", timeout=20)
        print("Restart:", o4.read().decode("utf-8", errors="replace"))
    finally:
        c.close()

if __name__ == "__main__":
    main()
