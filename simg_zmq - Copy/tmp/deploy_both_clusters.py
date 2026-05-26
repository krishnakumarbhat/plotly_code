#!/usr/bin/env python3
"""Upload pyz to both clusters, kill old brokers, restart."""
import paramiko, time

TARGETS = [
    {
        'host': '10.192.224.131',
        'remote_path': '/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz',
        'run_dir': '/mnt/usmidet/projects/RADARCORE/2-Sim/all_service',
        'label': 'Southfield',
        'start_cmd': "tmux send-keys -t 0 'cd /mnt/usmidet/projects/RADARCORE/2-Sim/all_service && ./run_hpcc_stack.sh' Enter",
        'broker_port': '9100',
    },
    {
        'host': '10.214.45.45',
        'remote_path': '/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/hpcc_main.pyz',
        'run_dir': '/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2',
        'label': 'Krakow all_service2',
        'start_cmd': "tmux new-window -t 0 -n as2 'cd /net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2 && ./run_hpcc_stack.sh' 2>/dev/null || true",
        'broker_port': '9100',
    },
]

LOCAL_PYZ = 'simg_sh_hpcc/hpcc_main.pyz'
USER = 'ouymc2'
PASSWORD = 'Zalikapope@202425'


def run(c, cmd, timeout=30):
    _, stdout, stderr = c.exec_command(cmd, timeout=timeout)
    out = stdout.read().decode().strip()
    err = stderr.read().decode().strip()
    return out, err


for t in TARGETS:
    print(f'\n=== {t["label"]} ===')
    c = paramiko.SSHClient()
    c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    c.connect(t['host'], username=USER, password=PASSWORD, timeout=15)

    sftp = c.open_sftp()
    sftp.put(LOCAL_PYZ, t['remote_path'])
    sftp.close()
    print(f'  Uploaded -> {t["remote_path"]}')

    rp = t['remote_path']
    out, _ = run(c, f"python3 - <<'PY'\nimport zipfile\nz = zipfile.ZipFile('{rp}')\nsrc = z.read('hpcc_main.py').decode()\nprint('OK: chmod+ssh found' if 'chmod' in src and 'ssh_run_as_user' in src else 'MISSING')\nPY")
    print(f'  Verify: {out}')

    out, _ = run(c, "pkill -f 'python.*hpcc_main.pyz.*broker-only' 2>/dev/null; echo done", timeout=10)
    print(f'  Kill old broker: {out}')
    time.sleep(3)

    out, _ = run(c, t['start_cmd'], timeout=10)
    print(f'  Restart sent')
    time.sleep(18)

    out, _ = run(c, "ps aux | grep 'hpcc_main.pyz.*broker-only' | grep -v grep", timeout=10)
    pid = out.split()[1] if out else '?'
    print(f'  Broker: pid={pid}')

    out, _ = run(c, f"ss -tlnp | grep {t['broker_port']}", timeout=10)
    print(f'  Port {t["broker_port"]}: {out or "(not yet)"}')

    c.close()

print('\nAll done.')
