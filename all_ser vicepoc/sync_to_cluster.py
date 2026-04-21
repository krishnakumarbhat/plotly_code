#!/usr/bin/env python3
"""
sync_to_cluster.py

Simple sync tool that uploads files from a local folder to a remote cluster
path over SFTP (Paramiko). It compares file size and mtime and only uploads
when changed. It is intended to be run on your laptop and push files to the
cluster at 10.214.45.45 (username: ouymc2). You will be prompted for the
SSH password when the script runs.

Usage:
  python sync_to_cluster.py \
    --local ./project \
    --host 10.214.45.45 \
    --user ouymc2 \
    --remote "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service"

Requirements:
  pip install paramiko

Notes:
 - Run this locally (on laptop) to push files to the cluster. It will not
   be auto-triggered by the .sif/.simg file on the cluster. If you need the
   cluster to run something on update, create a small wrapper script or
   schedule a job on the cluster that runs the container image.
"""

import argparse
import getpass
import os
import stat
import sys
from pathlib import Path

try:
    import paramiko
except Exception:
    print("This script requires paramiko. Install with: pip install paramiko")
    raise


def mkdirs_remote(sftp, remote_directory):
    """Ensure remote directory exists (create intermediate directories)."""
    dirs = []
    head = remote_directory
    while head not in ('', '/', '.'):
        try:
            sftp.stat(head)
            break
        except IOError:
            dirs.append(head)
            head = os.path.dirname(head)
    for d in reversed(dirs):
        try:
            sftp.mkdir(d)
        except Exception:
            pass


def remote_path_join(*parts):
    # ensure posix style path for remote
    return '/'.join(p.strip('/').replace('\\', '/') for p in parts if p != '')


def should_upload(sftp, local_path: Path, remote_path: str) -> bool:
    try:
        rstat = sftp.stat(remote_path)
    except IOError:
        return True
    # compare size and mtime (allow small delta)
    lsize = local_path.stat().st_size
    rsize = rstat.st_size
    if lsize != rsize:
        return True
    lmtime = int(local_path.stat().st_mtime)
    rmtime = int(rstat.st_mtime)
    if abs(lmtime - rmtime) > 1:
        return True
    return False


def upload_dir(sftp, local_root: Path, remote_root: str, delete_remote=False):
    local_root = local_root.resolve()
    uploaded = []
    for root, dirs, files in os.walk(local_root):
        rel = os.path.relpath(root, local_root)
        if rel == '.':
            rel = ''
        remote_dir = remote_path_join(remote_root, rel)
        mkdirs_remote(sftp, remote_dir)
        for fname in files:
            local_file = Path(root) / fname
            remote_file = remote_path_join(remote_dir, fname)
            if should_upload(sftp, local_file, remote_file):
                print(f"Uploading: {local_file} -> {remote_file}")
                sftp.put(str(local_file), remote_file)
                # try to set remote mtime to local mtime
                try:
                    atime = int(local_file.stat().st_atime)
                    mtime = int(local_file.stat().st_mtime)
                    sftp.utime(remote_file, (atime, mtime))
                except Exception:
                    pass
                uploaded.append(remote_file)
            else:
                print(f"Up-to-date: {local_file}")
    if delete_remote:
        # optional: delete remote files that don't exist locally
        print("Delete-remote option is enabled: scanning remote for deletions...")
        # simple implementation: walk remote tree under remote_root and remove files
        def remote_walk(path):
            try:
                for entry in sftp.listdir_attr(path):
                    mode = entry.st_mode
                    name = entry.filename
                    rpath = remote_path_join(path, name)
                    if stat.S_ISDIR(mode):
                        yield from remote_walk(rpath)
                    else:
                        yield rpath
            except IOError:
                return

        local_files = set()
        for root, _, files in os.walk(local_root):
            rel = os.path.relpath(root, local_root)
            if rel == '.':
                rel = ''
            for f in files:
                local_files.add(remote_path_join(remote_root, rel, f))

        for rfile in remote_walk(remote_root):
            if rfile not in local_files:
                print(f"Deleting remote file (not found locally): {rfile}")
                try:
                    sftp.remove(rfile)
                except Exception as e:
                    print("Failed to delete", rfile, e)

    return uploaded


def parse_args():
    p = argparse.ArgumentParser(description='Sync local folder to cluster via SFTP')
    p.add_argument('--local', '-l', default='project', help='Local folder to sync')
    p.add_argument('--host', required=True, help='Cluster host or IP')
    p.add_argument('--user', default=getpass.getuser(), help='Remote username')
    p.add_argument('--remote', '-r', required=True, help='Remote absolute path to sync into')
    p.add_argument('--port', type=int, default=22, help='SSH port')
    p.add_argument('--delete', action='store_true', help='Delete remote files not present locally')
    p.add_argument('--key', help='Path to private key file (optional)')
    p.add_argument('--skip', nargs='*', default=['.git'], help='List of directory names to skip')
    return p.parse_args()


def main():
    args = parse_args()
    local = Path(args.local)
    if not local.exists():
        print('Local path does not exist:', local)
        sys.exit(1)

    passwd = None
    pkey = None
    if args.key:
        try:
            pkey = paramiko.RSAKey.from_private_key_file(args.key)
        except Exception as e:
            print('Failed to load private key:', e)
            sys.exit(1)
    else:
        # ask for password interactively
        passwd = getpass.getpass(f"Password for {args.user}@{args.host}: ")

    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        if pkey:
            ssh.connect(args.host, port=args.port, username=args.user, pkey=pkey)
        else:
            ssh.connect(args.host, port=args.port, username=args.user, password=passwd)
    except Exception as e:
        print('SSH connection failed:', e)
        sys.exit(1)

    sftp = ssh.open_sftp()
    try:
        print('Starting upload...')
        uploaded = upload_dir(sftp, local, args.remote, delete_remote=args.delete)
        print('Upload complete. Files uploaded:', len(uploaded))
    finally:
        sftp.close()
        ssh.close()


if __name__ == '__main__':
    main()
