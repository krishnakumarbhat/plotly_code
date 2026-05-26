"""Run this on Southfield via copilot_remote_exec to extract and show _resolve_output_path."""
import zipfile
import sys
import os

PYZ = "/mnt/usmidet/projects/RADARCORE/2-Sim/all_service/hpcc_main.pyz"
PYZ_KRAKOW = "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service2/hpcc_main.pyz"
PYZ_KRAKOW_OLD = "/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/hpcc_main.pyz"

def show_func(pyz_path, label):
    print(f"\n===== {label} ({pyz_path}) =====")
    if not os.path.exists(pyz_path):
        print(f"  NOT FOUND")
        return
    try:
        with zipfile.ZipFile(pyz_path) as z:
            names = z.namelist()
            target = next((n for n in names if n.endswith("hpcc_main.py")), None)
            if not target:
                print(f"  hpcc_main.py not in zip, files: {names[:10]}")
                return
            content = z.read(target).decode("utf-8", errors="replace")
            lines = content.splitlines()
            # Find _resolve_output_path function
            start = None
            for i, line in enumerate(lines):
                if "def _resolve_output_path" in line:
                    start = i
                    break
            if start is None:
                print("  _resolve_output_path not found")
                return
            # Print up to 35 lines of the function
            for i in range(start, min(start + 35, len(lines))):
                print(f"  {i+1}: {lines[i]}")
                if i > start and lines[i].strip().startswith("def "):
                    break
    except Exception as e:
        print(f"  ERROR: {e}")

show_func(PYZ, "SOUTHFIELD")
show_func(PYZ_KRAKOW, "KRAKOW new (all_service2)")
show_func(PYZ_KRAKOW_OLD, "KRAKOW old (all_service) [reference]")
