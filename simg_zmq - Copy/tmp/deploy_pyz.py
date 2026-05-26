"""Upload new hpcc_main.pyz to Southfield, backup old one, verify content."""
from __future__ import annotations
import argparse, sys, io, zipfile, paramiko

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--password", required=True)
    p.add_argument("--pyz", required=True)
    p.add_argument("--host", default="10.192.224.131")
    p.add_argument("--user", default="ouymc2")
    args = p.parse_args()

    c = paramiko.SSHClient()
    c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    c.connect(args.host, username=args.user, password=args.password,
              look_for_keys=False, allow_agent=False, timeout=20)
    try:
        remote_path = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz"
        backup_path = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz.bak_pre_fix"

        # Backup old pyz
        _, o, e = c.exec_command(f"cp {remote_path} {backup_path} && echo 'backup ok'")
        print("Backup:", o.read().decode().strip(), e.read().decode().strip()[:100])

        # Upload new pyz
        sftp = c.open_sftp()
        sftp.put(args.pyz, remote_path)
        sftp.chmod(remote_path, 0o755)
        sftp.close()
        print("Upload: done")

        # Verify the fix is in the new pyz
        verify_script = """
import zipfile
with zipfile.ZipFile("/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz") as z:
    content = z.read("hpcc_main.py").decode()
    lines = content.splitlines()
    for i, line in enumerate(lines):
        if "def _resolve_output_path" in line:
            for j in range(i, min(i+20, len(lines))):
                print(str(j+1)+": "+lines[j])
                if j > i and lines[j].strip().startswith("def "):
                    break
            break
    # Check mkdir_step presence
    if "mkdir_step" in content:
        print("OK: mkdir_step found in launcher code")
    else:
        print("WARNING: mkdir_step NOT found")
    if "os.makedirs(candidate" in content:
        print("ERROR: old makedirs test still present!")
    else:
        print("OK: old makedirs test removed")
"""
        sftp2 = c.open_sftp()
        sftp2.putfo(io.BytesIO(verify_script.encode()), "/tmp/_verify_deploy.py")
        sftp2.close()
        _, o2, e2 = c.exec_command("python3 /tmp/_verify_deploy.py", timeout=20)
        print(o2.read().decode("utf-8", errors="replace"))
        err = e2.read().decode("utf-8", errors="replace")
        if err.strip():
            print("STDERR:", err[:300])
    finally:
        c.close()

if __name__ == "__main__":
    main()
