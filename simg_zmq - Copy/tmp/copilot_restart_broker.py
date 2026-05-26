"""Restart hpcc broker on Southfield."""
from __future__ import annotations
import argparse, sys, io, paramiko

RESTART = """
import subprocess, os, time, signal

# Find broker process (hpcc_main.pyz)
r = subprocess.Popen(
    ["pgrep", "-f", "hpcc_main.pyz"],
    stdout=subprocess.PIPE, stderr=subprocess.PIPE
)
out, err = r.communicate()
pids = [int(x.strip()) for x in out.decode().splitlines() if x.strip().isdigit()]
print("Current broker PIDs:", pids)

if pids:
    for pid in pids:
        try:
            os.kill(pid, signal.SIGTERM)
            print("Sent SIGTERM to", pid)
        except Exception as e:
            print("kill error:", e)
    time.sleep(3)
    # Check if still running
    r2 = subprocess.Popen(
        ["pgrep", "-f", "hpcc_main.pyz"],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    out2, _ = r2.communicate()
    remaining = [x.strip() for x in out2.decode().splitlines() if x.strip()]
    if remaining:
        print("Still running after SIGTERM, sending SIGKILL:", remaining)
        for p in remaining:
            try:
                os.kill(int(p), signal.SIGKILL)
            except:
                pass
    time.sleep(1)
    print("Broker stopped.")
else:
    print("No broker process found.")

# Find main_hpcc.sh process
r3 = subprocess.Popen(
    ["pgrep", "-af", "main_hpcc"],
    stdout=subprocess.PIPE, stderr=subprocess.PIPE
)
out3, _ = r3.communicate()
print("main_hpcc processes:", out3.decode().strip()[:300])

print("Done - broker restart complete. main_hpcc.sh will auto-restart the broker.")
"""

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
        sftp.putfo(io.BytesIO(RESTART.encode()), "/tmp/_restart_broker.py")
        sftp.close()
        _, o, e = c.exec_command("python3 /tmp/_restart_broker.py", timeout=30)
        sys.stdout.write(o.read().decode("utf-8", errors="replace"))
        err = e.read().decode("utf-8", errors="replace")
        if err.strip():
            sys.stderr.write(err)
    finally:
        c.close()

if __name__ == "__main__":
    main()
