#!/usr/bin/env python3
"""Deploy pyz to Krakow all_service2 and restart broker."""
import paramiko, time

REMOTE_PATH = '/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/hpcc_main.pyz'
RUN_DIR = '/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2'
LOCAL_PYZ = 'simg_sh_hpcc/hpcc_main.pyz'

c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect('10.214.45.45', username='ouymc2', password='Zalikapope@202425', timeout=15)

sftp = c.open_sftp()
sftp.put(LOCAL_PYZ, REMOTE_PATH)
sftp.close()
print('Upload done:', REMOTE_PATH)

def run(cmd):
    _, stdout, stderr = c.exec_command(cmd)
    out = stdout.read().decode().strip()
    err = stderr.read().decode().strip()
    return out, err

verify_cmd = "python3 - <<'PY'\nimport zipfile\nz = zipfile.ZipFile('%s')\nsrc = z.read('hpcc_main.py').decode()\nprint('OK: ssh_run_as_user found' if 'ssh_run_as_user' in src else 'MISSING')\nPY" % REMOTE_PATH
out, err = run(verify_cmd)
print('Verify:', out or err)

# Kill existing all_service2 broker
out, _ = run("pkill -f 'python.*hpcc_main.pyz.*broker-only' 2>/dev/null; sleep 2; echo killed")
print('Kill:', out)

# Start via new tmux window so it doesn't conflict with tmux session 0 (all_service)
out, err = run("tmux new-window -t 0 -n as2 'cd %s && ./run_hpcc_stack.sh' 2>/dev/null || tmux new-session -d -s as2 'cd %s && ./run_hpcc_stack.sh' 2>/dev/null; echo sent" % (RUN_DIR, RUN_DIR))
print('Start:', out or err)

time.sleep(18)

out, _ = run("ps aux | grep 'hpcc_main.pyz.*broker-only' | grep -v grep")
print('Broker processes:', out or '(none seen yet)')

out, _ = run("ss -tlnp | grep -E '9100|9101|5001|5002'")
print('Ports:', out)

c.close()
print('Done.')
