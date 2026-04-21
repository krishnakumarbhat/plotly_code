#!/usr/bin/env python3
"""
Parallel upload helper: splits a large file, uploads parts in parallel via scp,
concatenates on remote host, verifies checksum, cleans up parts.

Run in WSL:
  python3 parallel_copy_image.py --file ./all_services.simg --host 10.214.45.45 --user ouymc2 --dest /net/8k3/.../all_services --workers 6 --chunk 200
"""
import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import posixpath
import time
try:
    import paramiko
except Exception:
    paramiko = None

def md5sum(path):
    h = hashlib.md5()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()

def run(cmd, **kwargs):
    # Support both list and shell string commands
    if isinstance(cmd, (list, tuple)):
        print("+", " ".join(cmd))
    else:
        print("+", str(cmd))
    return subprocess.run(cmd, **kwargs)

def scp_upload(chunk_path, host, user, dest_dir, port):
    basename = os.path.basename(chunk_path)
    # Ensure remote path uses POSIX separators
    remote_path = posixpath.join(dest_dir.replace('\\', '/'), basename)
    remote = f"{user}@{host}:{remote_path}"
    cmd = ["scp"]
    if port:
        cmd += ["-P", str(port)]
    cmd += [chunk_path, remote]
    return run(cmd, check=True)


def paramiko_upload_chunks(chunks, host, user, dest_dir, port, password=None):
    """Upload chunks using a single Paramiko SFTP session."""
    if paramiko is None:
        raise RuntimeError('paramiko is not installed; install paramiko to use --use-paramiko')

    # Create SSH client and SFTP
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    connect_kwargs = {
        'hostname': host,
        'username': user,
        'port': port,
    }
    if password:
        connect_kwargs['password'] = password

    print('Connecting to', host)
    ssh.connect(**connect_kwargs)
    sftp = ssh.open_sftp()

    def _upload(chunk_path):
        basename = os.path.basename(chunk_path)
        remote_path = posixpath.join(dest_dir.replace('\\', '/'), basename)
        # Ensure the local chunk exists; sometimes transient filesystem issues
        # cause a brief MissingFile error. Retry a few times before failing.
        attempts = 5
        for attempt in range(1, attempts + 1):
            if os.path.exists(chunk_path):
                try:
                    print('SFTP put:', chunk_path, '->', remote_path)
                    sftp.put(chunk_path, remote_path)
                    return
                except Exception as e:
                    print(f'SFTP put error for {chunk_path} (attempt {attempt}):', e)
                    if attempt == attempts:
                        raise
            else:
                print(f'Local chunk missing, retrying ({attempt}/{attempts}):', chunk_path)
            time.sleep(0.5)
        # If we exit loop without returning, raise
        raise FileNotFoundError(chunk_path)

    success = False
    try:
        with ThreadPoolExecutor(max_workers=min(8, len(chunks))) as ex:
            futures = {ex.submit(_upload, c): c for c in chunks}
            for fut in as_completed(futures):
                chunk = futures[fut]
                try:
                    fut.result()
                    print('Uploaded:', os.path.basename(chunk))
                except Exception as e:
                    print('SFTP upload failed for', chunk, e)
                    raise
    finally:
        try:
            sftp.close()
        except Exception:
            pass
        try:
            ssh.close()
        except Exception:
            pass

def ssh_exec(host, user, port, remote_cmd):
    ssh = ["ssh"]
    if port:
        ssh += ["-p", str(port)]
    ssh += [f"{user}@{host}", remote_cmd]
    return run(ssh, check=True, shell=False)

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--file", required=True, help="Local file to transfer")
    p.add_argument("--host", required=False, help="Remote host (not required for --test-split)")
    p.add_argument("--user", required=False, help="Remote user (not required for --test-split)")
    p.add_argument("--dest", required=False, help="Remote destination directory (must exist). Not required for --test-split")
    p.add_argument('--test-split', action='store_true', help='Only split locally and list chunks (no upload)')
    p.add_argument("--port", type=int, default=22)
    p.add_argument("--chunk", type=int, default=200, help="Chunk size in MB")
    p.add_argument("--workers", type=int, default=4, help="Parallel uploads")
    p.add_argument("--prefix", default="part_", help="Remote/local chunk prefix")
    p.add_argument('--use-paramiko', action='store_true', help='Use paramiko SFTP for single-auth uploads')
    args = p.parse_args()

    src = os.path.abspath(args.file)
    if not os.path.isfile(src):
        print("ERROR: file not found:", src)
        sys.exit(2)

    orig_md5 = md5sum(src)
    size_mb = os.path.getsize(src) / (1024*1024)
    print(f"File: {src} ({size_mb:.1f} MB) md5: {orig_md5}")

    tmpdir = tempfile.mkdtemp(prefix="pcopy_")
    try:
        prefix_local = os.path.join(tmpdir, args.prefix)
        # Use system 'split' when available (WSL/Linux/Mac). On Windows, fall back
        # to a pure-Python splitter so users don't need external utilities.
        split_size_bytes = args.chunk * 1024 * 1024
        split_used = False
        try:
            # Try system split
            split_size = f"{args.chunk}M"
            print("Attempting to use system 'split' utility for speed...")
            run(["split", "-b", split_size, src, prefix_local], check=True)
            split_used = True
        except Exception as e:
            print("System 'split' not available or failed, falling back to Python splitter:", e)
            # Python-based split
            with open(src, 'rb') as f_in:
                idx = 0
                while True:
                    chunk = f_in.read(split_size_bytes)
                    if not chunk:
                        break
                    part_name = f"{args.prefix}{idx:04d}"
                    part_path = os.path.join(tmpdir, part_name)
                    with open(part_path, 'wb') as pf:
                        pf.write(chunk)
                    idx += 1

        # Discover chunk files (only those starting with prefix)
        chunks = sorted([
            os.path.join(tmpdir, n) for n in os.listdir(tmpdir)
            if n.startswith(os.path.basename(args.prefix))
        ])
        print("Chunks:", len(chunks))

        # If test-split mode, just print chunks and exit
        if args.test_split:
            print('Split-only mode: listing chunks:')
            for c in chunks:
                print('  ', c, os.path.getsize(c))
            return

        # Upload in parallel
        if not (args.host and args.user and args.dest):
            raise SystemExit('Host, user and dest are required unless --test-split is used')

        # Upload using paramiko (single auth) or scp
        if args.use_paramiko:
            if paramiko is None:
                raise SystemExit('paramiko not installed; please install paramiko or omit --use-paramiko')
            import getpass
            passwd = getpass.getpass(f"Password for {args.user}@{args.host}: ")
            paramiko_upload_chunks(chunks, args.host, args.user, args.dest, args.port, password=passwd)
        else:
            with ThreadPoolExecutor(max_workers=args.workers) as ex:
                futures = {ex.submit(scp_upload, c, args.host, args.user, args.dest, args.port): c for c in chunks}
                for fut in as_completed(futures):
                    chunk = futures[fut]
                    try:
                        fut.result()
                        print("Uploaded:", os.path.basename(chunk))
                    except subprocess.CalledProcessError as e:
                        print("Upload failed for", chunk, e)
                        raise

        # Concatenate remotely
        remote_prefix = os.path.join(args.dest, args.prefix)
        remote_target = os.path.join(args.dest, os.path.basename(src))
        cat_cmd = f"cat {remote_prefix}* > {remote_target} && rm -f {remote_prefix}*"
        print("Concatenating on remote:", remote_target)
        ssh_exec(args.host, args.user, args.port, cat_cmd)

        # Verify remote md5
        remote_md5_cmd = f"md5sum {remote_target} | awk '{{print $1}}'"
        print("Verifying remote checksum...")
        proc = subprocess.run(
            ["ssh", "-p", str(args.port), f"{args.user}@{args.host}", remote_md5_cmd],
            capture_output=True, text=True, check=True
        )
        remote_md5 = proc.stdout.strip()
        print("Remote md5:", remote_md5)
        if remote_md5 != orig_md5:
            print("ERROR: checksum mismatch!")
            raise RuntimeError('checksum mismatch')
        print("SUCCESS: file uploaded and verified.")
        success = True

    finally:
        if success:
            print("Cleaning up local chunks...")
            shutil.rmtree(tmpdir, ignore_errors=True)
        else:
            print(f"Upload failed; preserving chunks in {tmpdir} for inspection.")

if __name__ == "__main__":
    # Allow a quick split-only test to validate behavior on Windows
    import sys as _sys
    if '--test-split' in _sys.argv:
        # Example: python parallel_copy_image.py --test-split --file small.bin --chunk 1
        main()
    else:
        main()
    