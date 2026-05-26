"""
Submit a test KPI job to the broker to verify the output path fix.
Uses 2 sample HDF files and a writable output dir under RADARCORE (ouymc2 can write there).
Also verifies that the new launcher script contains mkdir -p.
"""
from __future__ import annotations
import argparse, sys, io, json, socket, time, paramiko

BROKER_HOST = "10.192.224.131"
BROKER_PORT = 9100

# Two sample input HDF files from pcmzxl's run (readable by ouymc2 from GPO-IFV7XX/4-Checkout)
# Use a small subset - just 2 files to keep it quick
SAMPLE_JSON_PATH = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/runtime_state/test_copilot_sample.json"
# Use ouymc2's own writable space as test output
TEST_OUTPUT_DIR = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/runs/ouymc2/copilot_test_output"

PREPARE_SCRIPT = f"""
import os, json, glob

# Create a minimal test mudp.json with 2 HDF files
# Look for actual HDF files to use
hdf_base = "/mnt/usmidet/projects/GPO-IFV7XX"
sample_hdfs = []
try:
    for root, dirs, files in os.walk("/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/runs"):
        for f in files:
            if f.endswith(".json") and "mudp" in f.lower():
                fpath = os.path.join(root, f)
                try:
                    with open(fpath) as fh:
                        d = json.load(fh)
                    if d.get("INPUT_HDF") and len(d["INPUT_HDF"]) >= 2:
                        inp = d["INPUT_HDF"][:2]
                        out = [p.replace("_b05.h5", "_b05_r000801151.h5") for p in inp]
                        sample = {{"INPUT_HDF": inp, "OUTPUT_HDF": out}}
                        with open("{SAMPLE_JSON_PATH}", "w") as fw:
                            json.dump(sample, fw, indent=2)
                        print("Created test json from:", fpath)
                        print("Input files:", inp)
                        break
                except:
                    pass
        if os.path.exists("{SAMPLE_JSON_PATH}"):
            break
except Exception as e:
    print("walk error:", e)

if not os.path.exists("{SAMPLE_JSON_PATH}"):
    # Create a minimal stub
    stub = {{
        "INPUT_HDF": [
            "/mnt/usmidet/projects/GPO-IFV7XX/4-Checkout/dummy_0000_b05.h5",
            "/mnt/usmidet/projects/GPO-IFV7XX/4-Checkout/dummy_0001_b05.h5"
        ],
        "OUTPUT_HDF": [
            "/mnt/usmidet/projects/GPO-IFV7XX/4-Checkout/dummy_0000_b05_r000801151.h5",
            "/mnt/usmidet/projects/GPO-IFV7XX/4-Checkout/dummy_0001_b05_r000801151.h5"
        ]
    }}
    os.makedirs(os.path.dirname("{SAMPLE_JSON_PATH}"), exist_ok=True)
    with open("{SAMPLE_JSON_PATH}", "w") as fw:
        json.dump(stub, fw, indent=2)
    print("Created stub test json")

# Create test output dir to confirm it's writable by ouymc2
os.makedirs("{TEST_OUTPUT_DIR}", exist_ok=True)
print("Output dir created:", "{TEST_OUTPUT_DIR}")
print("READY")
"""


def send_broker_request(host, port, payload):
    with socket.create_connection((host, port), timeout=30) as s:
        s.sendall((json.dumps(payload) + "\n").encode("utf-8"))
        data = b""
        while True:
            chunk = s.recv(4096)
            if not chunk:
                break
            data += chunk
            if b"\n" in data:
                break
    return json.loads(data.decode("utf-8").strip())


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--password", required=True)
    p.add_argument("--host", default="10.192.224.131")
    p.add_argument("--user", default="ouymc2")
    args = p.parse_args()

    # Step 1: Prepare test json on remote
    c = paramiko.SSHClient()
    c.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    c.connect(args.host, username=args.user, password=args.password,
              look_for_keys=False, allow_agent=False, timeout=20)
    try:
        sftp = c.open_sftp()
        sftp.putfo(io.BytesIO(PREPARE_SCRIPT.encode()), "/tmp/_prepare_test.py")
        sftp.close()
        _, o, e = c.exec_command("python3 /tmp/_prepare_test.py", timeout=20)
        print("=== Prepare ===")
        print(o.read().decode("utf-8", errors="replace"))
        err = e.read().decode("utf-8", errors="replace")
        if err.strip():
            print("STDERR:", err[:200])
    finally:
        c.close()

    # Step 2: Submit job to broker with explicit output dir (ouymc2's own writable space)
    print()
    print("=== Submit test job to broker ===")
    print(f"Output dir: {TEST_OUTPUT_DIR}")
    print(f"JSON path: {SAMPLE_JSON_PATH}")

    payload = {
        "action": "submit",
        "user": "ouymc2",
        "tool_key": "udp_kpi",
        "paths": {
            "json_path": SAMPLE_JSON_PATH,
            "output_dir": TEST_OUTPUT_DIR,
            "input_mode": "json",
            "interactive_plot_mode": "disabled",
        },
        "resources": {
            "scheduler": "slurm",
            "account": "radarcore",
            "partition": "defq",
            "cpus": 8,
            "memory": "32G",
            "time_limit": "00:30:00",
        }
    }

    try:
        response = send_broker_request(BROKER_HOST, BROKER_PORT, payload)
        print("Broker response:", json.dumps(response, indent=2))
    except Exception as e:
        print(f"Broker connection error: {e}")
        print("Trying to check if broker is up...")
        return

    # Step 3: Check the generated launcher script to verify mkdir -p is there
    if response.get("ok"):
        run_id = response.get("run_id") or response.get("id")
        print()
        print("=== Check launcher script for mkdir -p ===")

        time.sleep(2)
        c2 = paramiko.SSHClient()
        c2.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        c2.connect(args.host, username=args.user, password=args.password,
                   look_for_keys=False, allow_agent=False, timeout=20)
        try:
            check_cmd = f"find /mnt/usmidet/projects/RADARCORE/2-Sim/all_service/runs/ouymc2 -name 'slurm_tmux_launcher.sh' -newer /tmp/_prepare_test.py 2>/dev/null | head -3 | xargs -I{{}} bash -c 'echo === {{}} ===; grep -n \"mkdir -p\" {{}} | head -5'"
            _, o3, _ = c2.exec_command(check_cmd, timeout=15)
            print(o3.read().decode("utf-8", errors="replace"))
        finally:
            c2.close()


if __name__ == "__main__":
    main()
