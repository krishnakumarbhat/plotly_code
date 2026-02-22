#!/usr/bin/env python3
import json
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
import subprocess

def list_hdf_files(folder: Path):
    return sorted(folder.rglob("*.h5"))

def merge_no_duplicates(existing, new):
    seen = set(existing)
    result = list(existing)
    for item in new:
        if item not in seen:
            result.append(item)
            seen.add(item)
    return result

def build_paired_paths(hin_dir: Path, hout_dir: Path):
    in_files = list_hdf_files(hin_dir)
    out_files = list_hdf_files(hout_dir)
    out_index = {f.name: f for f in out_files}

    input_paths = []
    output_paths = []
    for in_f in in_files:
        out_f = out_index.get(in_f.name)
        if out_f:
            input_paths.append(str(in_f.resolve()))
            output_paths.append(str(out_f.resolve()))
    return input_paths, output_paths

def read_config_from_xml(xml_path: Path):
    try:
        tree = ET.parse(xml_path)
        root = tree.getroot()
    except Exception as e:
        print(f"ERROR: Failed to read XML config: {e}", file=sys.stderr)
        sys.exit(1)

    def get(tag):
        elem = root.find(tag)
        if elem is None or not elem.text:
            print(f"ERROR: Missing or empty <{tag}> in input.xml", file=sys.stderr)
            sys.exit(1)
        return elem.text.strip()

    return {
        "input_hdf": Path(get("InputHDF_Folder")),
        "output_hdf": Path(get("OutputHDF_Folder")),
        "input_json": Path(get("InputJSON_Path")),
        "html_config": Path(get("HTML_Config_Path")),
        "singularity_image": Path(get("Singularity_Image_Path")),
        "output_path": Path(get("Output_Path")),
    }

def run_with_module_load(module_name: str, command: str):
    chained = f"module load {module_name} && {command}"
    try:
        subprocess.run(["bash", "-lc", chained], check=True)
    except subprocess.CalledProcessError as e:
        print("\n Command failed.", file=sys.stderr)
        print(f"Exit code: {e.returncode}", file=sys.stderr)
        sys.exit(e.returncode)
    except FileNotFoundError:
        print("ERROR: 'bash' not found. This script requires Bash to load modules.", file=sys.stderr)
        sys.exit(1)

def main():
    script_dir = Path(__file__).resolve().parent
    xml_path = script_dir / "input.xml"

    if not xml_path.exists():
        print(f"ERROR: input.xml not found at {xml_path}", file=sys.stderr)
        sys.exit(1)

    cfg = read_config_from_xml(xml_path)

    hin = cfg["input_hdf"]
    hout = cfg["output_hdf"]
    json_path = cfg["input_json"]
    html_config = cfg["html_config"]
    sif_image = cfg["singularity_image"]
    output_dir = cfg["output_path"]

    print("=== HTML Report Builder ===")
    print(f"Using config file: {xml_path}")

    if not hin.is_dir():
        print(f"ERROR: Input HDF folder not found: {hin}", file=sys.stderr)
        sys.exit(1)
    if not hout.is_dir():
        print(f"ERROR: Output HDF folder not found: {hout}", file=sys.stderr)
        sys.exit(1)
    if not json_path.exists():
        print(f"ERROR: JSON file not found: {json_path}", file=sys.stderr)
        sys.exit(1)
    if not html_config.exists():
        print(f"ERROR: HTML config not found: {html_config}", file=sys.stderr)
        sys.exit(1)
    if not sif_image.exists():
        print(f"ERROR: Singularity image not found: {sif_image}", file=sys.stderr)
        sys.exit(1)
    output_dir.mkdir(parents=True, exist_ok=True)

    try:
        data = json.loads(json_path.read_text(encoding="utf-8"))
    except Exception as e:
        print(f"ERROR: Could not parse JSON: {e}", file=sys.stderr)
        sys.exit(1)

    if "InputHDF" not in data or "OutputHDF" not in data:
        print("ERROR: JSON must contain 'InputHDF' and 'OutputHDF' arrays.", file=sys.stderr)
        sys.exit(1)
    if not isinstance(data["InputHDF"], list) or not isinstance(data["OutputHDF"], list):
        print("ERROR: 'InputHDF' and 'OutputHDF' must be arrays.", file=sys.stderr)
        sys.exit(1)

    new_input_paths, new_output_paths = build_paired_paths(hin, hout)
    data["InputHDF"] = merge_no_duplicates(data["InputHDF"], new_input_paths)
    data["OutputHDF"] = merge_no_duplicates(data["OutputHDF"], new_output_paths)

    json_path.write_text(json.dumps(data, indent=2), encoding="utf-8")

    print("\n inputs.json updated successfully")
    print(f"   InputHDF entries : {len(data['InputHDF'])}")
    print(f"   OutputHDF entries: {len(data['OutputHDF'])}")

    cmd = (
        f'singularity exec "{sif_image}" '
        f'/RUN_HTML.sh "{html_config}" "{json_path}" "{output_dir}"'
    )
    run_with_module_load("singularity/3.8.0", cmd)

    print("\n Singularity execution completed successfully.")

if __name__ == "__main__":
    main()