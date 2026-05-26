"""Upload probe script and run it via SSH+SFTP."""
from __future__ import annotations
import argparse
import sys
import io
import paramiko

PROBE = r'''
import zipfile, os

def show_func(pyz_path, label):
    print("===== " + label + " =====")
    if not os.path.exists(pyz_path):
        print("  NOT FOUND: " + pyz_path)
        return
    with zipfile.ZipFile(pyz_path) as z:
        names = z.namelist()
        target = next((n for n in names if n.endswith("hpcc_main.py")), None)
        if not target:
            print("  hpcc_main.py not in zip, files: " + str(names[:10]))
            return
        content = z.read(target).decode("utf-8", errors="replace")
        lines = content.splitlines()
        start = None
        for i, line in enumerate(lines):
            if "def _resolve_output_path" in line:
                start = i
                break
        if start is None:
            print("  _resolve_output_path not found")
            return
        end = min(start + 40, len(lines))
        for i in range(start, end):
            print("  " + str(i+1) + ": " + lines[i])
            if i > start and lines[i].strip().startswith("def "):
                break

show_func("/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz", "SOUTHFIELD")
show_func("/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/hpcc_main.pyz", "KRAKOW new (all_service2)")
show_func("/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/hpcc_main.pyz", "KRAKOW old (all_service) [reference]")
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
        sftp.putfo(io.BytesIO(PROBE.encode()), "/tmp/_copilot_probe.py")
        sftp.close()
        _, stdout, stderr = client.exec_command("python3 /tmp/_copilot_probe.py", timeout=30)
        sys.stdout.write(stdout.read().decode("utf-8", errors="replace"))
        err = stderr.read().decode("utf-8", errors="replace")
        if err.strip():
            sys.stderr.write(err)
    finally:
        client.close()
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
