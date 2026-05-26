#!/usr/bin/env python3
"""Test SSH-run-as-user mechanism: submit a job with user_password in payload,
verify launcher script contains ssh command, and that run_dir is chmod 777."""
import json, socket, time, pathlib

BROKER_HOST = '10.192.224.131'
BROKER_PORT = 9100

def broker_request(payload):
    data = (json.dumps(payload) + '\n').encode()
    with socket.create_connection((BROKER_HOST, BROKER_PORT), timeout=15) as s:
        s.sendall(data)
        return json.loads(s.recv(65536).decode())

# Check available tools via ping
resp = broker_request({'action': 'ping'})
tools = resp.get('tools', [])
print('Available tools:', [t['tool_key'] for t in tools])

tool_key = next((t['tool_key'] for t in tools if 'kpi' in t['tool_key'].lower() or 'udp' in t['tool_key'].lower()), None)
if not tool_key:
    print('No KPI tool found, using first tool:', tools[0]['tool_key'] if tools else 'NONE')
    tool_key = tools[0]['tool_key'] if tools else None

if not tool_key:
    print('ERROR: No tools registered')
    exit(1)

print(f'Using tool: {tool_key}')

# Submit with user_password (using ouymc2 to self-SSH as test)
payload = {
    'action': 'submit',
    'tool_key': tool_key,
    'user': 'ouymc2',
    'user_password': 'Zalikapope@202425',  # test: ouymc2 SSHes to itself
    'session_id': 'test_ssh_fix',
    'mode': 'hdf',
    'paths': {
        'output_dir': '/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/runs/ouymc2/ssh_fix_test_output',
        'input_mode': 'hdf',
        'input_hdf': '/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/runs/ouymc2/test.mf4',
        'output_hdf': '/tmp/test_output.hdf',
        'json_path': '',
        'config_xml': '',
    },
    'resources': {
        'scheduler': 'slurm',
        'partition': 'defq',
        'account': 'radarcore',
        'cpus': 1,
        'memory': '1G',
        'time_limit': '00:05:00',
    }
}

resp = broker_request(payload)
print('\nSubmit response:', json.dumps(resp, indent=2))

if not resp.get('ok'):
    print('ERROR submitting')
    exit(1)

job_id = resp['job_id']
print(f'\nJob ID: {job_id}')

time.sleep(2)
status_resp = broker_request({'action': 'status', 'runtime_job_id': job_id})
job = status_resp.get('job', {})
print('Job status:', job.get('status'))
print('Status detail:', job.get('status_detail', ''))

# Find the launcher script
import paramiko
c = paramiko.SSHClient()
c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
c.connect(BROKER_HOST, username='ouymc2', password='Zalikapope@202425', timeout=10)

_, stdout, _ = c.exec_command(f"find /mnt/usmidet/projects/RADARCORE/2-Sim/all_service/runs/ouymc2 -name 'slurm_tmux_launcher.sh' -newer /mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz 2>/dev/null | head -3")
launchers = stdout.read().decode().strip().split('\n')
print('\nLauncher scripts found:', launchers)

for launcher in launchers:
    if not launcher.strip():
        continue
    _, stdout, _ = c.exec_command(f"head -10 {launcher}")
    content = stdout.read().decode()
    print(f'\n--- {launcher} (first 10 lines) ---')
    print(content)
    
    # Check for SSH
    if 'ssh' in content.lower():
        print('  ✓ SSH command found in launcher')
    else:
        print('  ✗ SSH command NOT found in launcher')
    
    # Check run_dir permissions
    run_dir = pathlib.PurePosixPath(launcher).parent
    _, stdout, _ = c.exec_command(f"stat -c '%a %n' {run_dir}")
    perms = stdout.read().decode().strip()
    print(f'  run_dir perms: {perms}')
    if perms.startswith('777'):
        print('  ✓ run_dir is chmod 777')
    else:
        print('  ✗ run_dir is NOT chmod 777:', perms)

c.close()
print('\nTest complete.')
