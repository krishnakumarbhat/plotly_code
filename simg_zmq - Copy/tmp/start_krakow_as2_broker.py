#!/usr/bin/env python3
"""Start Krakow all_service2 broker directly via nohup, bypassing tmux."""
import paramiko, time

AS2_DIR = '/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2'
PYZ = f'{AS2_DIR}/hpcc_main.pyz'
LOG = f'{AS2_DIR}/logs/hpcc_broker.log'

c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect('10.214.45.45', username='ouymc2', password='Zalikapope@202425', timeout=15)


def run(cmd, timeout=15):
    _, stdout, stderr = c.exec_command(cmd, timeout=timeout)
    return stdout.read().decode().strip(), stderr.read().decode().strip()


# Kill any existing all_service2 brokers
out, _ = run("pkill -f 'python.*all_service2.*broker-only' 2>/dev/null; sleep 1; echo done")
print('Kill:', out)

# Check existing pyz size/date
out, _ = run(f"ls -la {PYZ}")
print('pyz:', out)

# Set up needed env vars and run broker directly in background
broker_cmd = (
    f'mkdir -p {AS2_DIR}/logs; '
    f'cd {AS2_DIR}; '
    f'source bundle_common.sh 2>/dev/null || true; '
    f'export HPCC_BUNDLE_ROOT={AS2_DIR}; '
    f'export BUNDLE_ROOT={AS2_DIR}; '
    f'export HPCC_PROJECT_ROOT={AS2_DIR}/bundle_src; '
    f'export HPCC_RUNTIME_DB={AS2_DIR}/runtime_state/main_html/.cache_html/hpc_tools_dev.db; '
    f'export HPCC_BROKER_HOST=0.0.0.0; '
    f'export HPCC_BROKER_PORT=9100; '
    f'export PYTHONPATH={AS2_DIR}; '
    f'nohup python3 {PYZ} --host 0.0.0.0 --port 9100 --broker-only '
    f'>> {AS2_DIR}/logs/hpcc_broker.log 2>&1 &'
)
out, err = run(broker_cmd)
print('Start out:', out)
print('Start err:', err)

time.sleep(8)

out, _ = run("ps aux | grep 'all_service2.*broker-only' | grep -v grep")
print('Broker ps:', out or '(none)')
out, _ = run("ss -tlnp | grep 9100")
print('Port 9100:', out or '(not listening)')
out, _ = run(f"tail -10 {LOG}")
print('Log tail:', out)

c.close()
print('Done.')
