#!/usr/bin/env python3
"""Deploy updated pyz to Southfield and Krakow all_service2, kill old broker, restart."""
import paramiko, time, sys

HOSTS = [
    {
        'host': '10.192.224.131',
        'user': 'ouymc2',
        'password': 'Zalikapope@202425',
        'remote_path': '/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz',
        'run_dir': '/mnt/usmidet/projects/RADARCORE/2-Sim/all_service',
        'label': 'Southfield',
    },
    {
        'host': '10.214.45.45',
        'user': 'ouymc2',
        'password': 'Zalikapope@202425',
        'remote_path': '/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/hpcc_main.pyz',
        'run_dir': '/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2',
        'label': 'Krakow all_service2',
    },
]

LOCAL_PYZ = 'simg_sh_hpcc/hpcc_main.pyz'


def run(client, cmd):
    _, stdout, stderr = client.exec_command(cmd)
    out = stdout.read().decode()
    err = stderr.read().decode()
    return out.strip(), err.strip()


def deploy(info):
    label = info['label']
    print(f'\n=== {label} ===')
    c = paramiko.SSHClient()
    c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    c.connect(info['host'], username=info['user'], password=info['password'], timeout=15)

    sftp = c.open_sftp()
    backup = info['remote_path'] + '.bak_ssh_fix'
    # backup existing
    run(c, f'cp {info["remote_path"]} {backup} 2>/dev/null || true')
    sftp.put(LOCAL_PYZ, info['remote_path'])
    sftp.close()
    print(f'  Uploaded {LOCAL_PYZ} -> {info["remote_path"]}')

    # verify SSH_ASKPASS pattern in pyz
    rp = info['remote_path']
    verify_cmd = f"python3 -c \"import zipfile; z=zipfile.ZipFile('{rp}'); src=z.read('hpcc_main.py').decode(); print('OK: ssh_run_as_user found' if 'ssh_run_as_user' in src else 'MISSING')\""
    out2, _ = run(c, verify_cmd)
    print(f'  Verify: {out2}')

    # kill existing broker
    out, _ = run(c, f'pkill -f "python.*hpcc_main.pyz.*broker-only" 2>/dev/null; echo killed')
    print(f'  Kill old broker: {out}')
    time.sleep(3)

    # restart via tmux in the correct directory
    tmux_cmd = f'tmux send-keys -t 0 "cd {info["run_dir"]} && ./run_hpcc_stack.sh" Enter'
    run(c, tmux_cmd)
    print(f'  Sent restart command to tmux session 0')
    time.sleep(15)

    # check broker is up
    out, _ = run(c, 'ps aux | grep "hpcc_main.pyz.*broker-only" | grep -v grep')
    if out:
        print(f'  Broker running: {out.split()[1]} (pid {out.split()[1]})')
        pid = out.split()[1]
        print(f'  PID: {pid}')
    else:
        print(f'  WARNING: broker not seen in ps')

    out, _ = run(c, 'ss -tlnp | grep -E "9100|9101|5001|5002"')
    print(f'  Ports: {out}')

    c.close()


for h in HOSTS:
    deploy(h)

print('\nDone.')
