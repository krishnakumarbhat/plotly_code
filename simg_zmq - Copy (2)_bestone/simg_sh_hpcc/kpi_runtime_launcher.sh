#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  kpi_runtime_launcher.sh \
    --target can_kpi|udp_kpi|interactive_plot \
    --source-target can_kpi|udp_kpi \
    --interactive-mode disabled|enabled|only \
    --input-mode json|hdf \
    --output-dir /path/to/output \
    [--json-path /path/to/input.json] \
    [--input-hdf /path/to/input.h5 --output-hdf /path/to/output.h5] \
    [--config-xml /path/to/config.xml] \
    [--optional-config /path/to/plot-config.json] \
    [--port 5560]

Behavior:
  disabled -> run only the selected KPI target
  enabled  -> run KPI plus Interactive Plot for the selected target
  only     -> run only Interactive Plot using the selected source profile
EOF
}

TARGET=""
SOURCE_TARGET="udp_kpi"
INTERACTIVE_MODE="disabled"
INPUT_MODE="json"
OUTPUT_DIR=""
JSON_PATH=""
INPUT_HDF=""
OUTPUT_HDF=""
CONFIG_XML=""
OPTIONAL_CONFIG=""
PORT="5560"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --target) TARGET="$2"; shift 2 ;;
    --source-target) SOURCE_TARGET="$2"; shift 2 ;;
    --interactive-mode) INTERACTIVE_MODE="$2"; shift 2 ;;
    --input-mode) INPUT_MODE="$2"; shift 2 ;;
    --output-dir) OUTPUT_DIR="$2"; shift 2 ;;
    --json-path) JSON_PATH="$2"; shift 2 ;;
    --input-hdf) INPUT_HDF="$2"; shift 2 ;;
    --output-hdf) OUTPUT_HDF="$2"; shift 2 ;;
    --config-xml) CONFIG_XML="$2"; shift 2 ;;
    --optional-config) OPTIONAL_CONFIG="$2"; shift 2 ;;
    --port) PORT="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "ERROR: Unknown argument: $1" >&2; usage; exit 1 ;;
  esac
done

if [[ -z "$TARGET" || -z "$OUTPUT_DIR" ]]; then
  usage
  exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
INTERACTIVE_SIMG="$SCRIPT_DIR/interactiveplot.simg"
UDP_KPI_SIMG="$SCRIPT_DIR/udp_kpi.simg"
CAN_KPI_SIMG="$SCRIPT_DIR/can_kpi.simg"
DEFAULT_MUDP_XML="$WORKSPACE_ROOT/KPI/intplot_kpi/ConfigInteractivePlots.xml"
DEFAULT_BORDNET_XML="$WORKSPACE_ROOT/KPI/intplot_kpi/ConfigInteractivePlots_bordnet.xml"

LOG_DIR="$OUTPUT_DIR/logs"
mkdir -p "$OUTPUT_DIR" "$LOG_DIR"

require_file() {
  local path="$1"
  local label="$2"
  if [[ ! -f "$path" ]]; then
    echo "ERROR: $label not found: $path" >&2
    exit 1
  fi
}

try_load_module() {
  local module_name="$1"
  if command -v module >/dev/null 2>&1; then
    module load "$module_name" >/dev/null 2>&1 || true
    return 0
  fi

  if [[ -f /etc/profile.d/modules.sh ]]; then
    # shellcheck disable=SC1091
    source /etc/profile.d/modules.sh >/dev/null 2>&1 || true
  fi
  if command -v module >/dev/null 2>&1; then
    module load "$module_name" >/dev/null 2>&1 || true
    return 0
  fi

  if command -v modulecmd >/dev/null 2>&1; then
    eval "$(modulecmd bash load "$module_name" 2>/dev/null)" || true
    return 0
  fi

  return 1
}

detect_container_runtime() {
  if command -v apptainer >/dev/null 2>&1; then
    echo "apptainer"
    return 0
  fi
  if command -v singularity >/dev/null 2>&1; then
    echo "singularity"
    return 0
  fi

  try_load_module apptainer || true
  try_load_module singularity || true
  try_load_module singularityce || true

  if command -v apptainer >/dev/null 2>&1; then
    echo "apptainer"
    return 0
  fi
  if command -v singularity >/dev/null 2>&1; then
    echo "singularity"
    return 0
  fi

  return 1
}

if ! CTR_CMD="$(detect_container_runtime)"; then
  echo "ERROR: Neither apptainer nor singularity is available in this shell." >&2
  exit 1
fi

CTR_BIND_OPTS=()
if [[ -d /mnt/c ]]; then
  CTR_BIND_OPTS+=(--bind /mnt/c:/mnt/c)
fi

require_file "$INTERACTIVE_SIMG" "Interactive Plot SIMG"
require_file "$UDP_KPI_SIMG" "UDP KPI SIMG"
require_file "$CAN_KPI_SIMG" "CAN KPI SIMG"

case "$SOURCE_TARGET" in
  can_kpi)
    SOURCE_SELECTION="BORDNET"
    DEFAULT_CONFIG_XML="$DEFAULT_BORDNET_XML"
    ;;
  udp_kpi|interactive_plot)
    SOURCE_SELECTION="MUDP"
    DEFAULT_CONFIG_XML="$DEFAULT_MUDP_XML"
    ;;
  *)
    echo "ERROR: Unsupported source target: $SOURCE_TARGET" >&2
    exit 1
    ;;
esac

if [[ -z "$CONFIG_XML" ]]; then
  CONFIG_XML="$DEFAULT_CONFIG_XML"
fi
require_file "$CONFIG_XML" "Interactive Plot XML config"

if [[ "$INPUT_MODE" == "json" ]]; then
  if [[ -z "$JSON_PATH" ]]; then
    echo "ERROR: --json-path is required for json mode" >&2
    exit 1
  fi
  require_file "$JSON_PATH" "Input JSON"
  INTERACTIVE_INPUT_JSON="$JSON_PATH"
else
  if [[ -z "$INPUT_HDF" || -z "$OUTPUT_HDF" ]]; then
    echo "ERROR: --input-hdf and --output-hdf are required for hdf mode" >&2
    exit 1
  fi
  require_file "$INPUT_HDF" "Input HDF"
  require_file "$OUTPUT_HDF" "Output HDF"
  INTERACTIVE_INPUT_JSON="$LOG_DIR/interactive_inputs.json"
  python3 - "$INPUT_HDF" "$OUTPUT_HDF" "$INTERACTIVE_INPUT_JSON" <<'PY'
import json
import sys

input_hdf, output_hdf, destination = sys.argv[1:4]
with open(destination, 'w', encoding='utf-8') as handle:
    json.dump({'INPUT_HDF': [input_hdf], 'OUTPUT_HDF': [output_hdf]}, handle, indent=2)
PY
fi

if [[ -n "$OPTIONAL_CONFIG" ]]; then
  require_file "$OPTIONAL_CONFIG" "Optional plot config"
fi

KPI_FLAG="0"
if [[ "$INTERACTIVE_MODE" == "enabled" && "$TARGET" == "udp_kpi" ]]; then
  KPI_FLAG="1"
fi

PREPARED_CONFIG_XML="$LOG_DIR/interactive_runtime_config.xml"
python3 - "$CONFIG_XML" "$PREPARED_CONFIG_XML" "$SOURCE_SELECTION" "$KPI_FLAG" <<'PY'
import sys
import xml.etree.ElementTree as ET

source_path, destination_path, source_selection, kpi_flag = sys.argv[1:5]
tree = ET.parse(source_path)
root = tree.getroot()

source_node = root.find('HDF_SOURCE_SELECTION')
if source_node is None:
    source_node = ET.SubElement(root, 'HDF_SOURCE_SELECTION')
source_node.text = source_selection

plot_mode = root.find('PLOT_MODE')
if plot_mode is None:
    plot_mode = ET.SubElement(root, 'PLOT_MODE')
kpi_node = plot_mode.find('KPI')
if kpi_node is None:
    kpi_node = ET.SubElement(plot_mode, 'KPI')
kpi_node.text = kpi_flag

tree.write(destination_path, encoding='utf-8', xml_declaration=True)
PY

run_interactive_plot() {
  local interactive_output_dir="$1"
  local interactive_log="$2"
  mkdir -p "$interactive_output_dir"

  local command=("$CTR_CMD" run)
  command+=("${CTR_BIND_OPTS[@]}")
  if [[ "$KPI_FLAG" == "1" ]]; then
    command+=(--env "KPI_SERVER_HOST=127.0.0.1,KPI_SERVER_PORT=$PORT")
  fi
  command+=("$INTERACTIVE_SIMG" "$PREPARED_CONFIG_XML" "$INTERACTIVE_INPUT_JSON" "$interactive_output_dir")
  if [[ -n "$OPTIONAL_CONFIG" ]]; then
    command+=("$OPTIONAL_CONFIG")
  fi

  "${command[@]}" >"$interactive_log" 2>&1
}

write_combined_index() {
  local index_path="$1"
  local title="$2"
  local can_link="$3"
  local interactive_link="$4"

  cat >"$index_path" <<EOF
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>$title</title>
  <style>
    body{font-family:Segoe UI,Arial,sans-serif;background:#f6f3ec;color:#23313a;padding:32px;}
    .card{max-width:760px;background:#fff;border:1px solid #d7ddd8;border-radius:20px;padding:24px;box-shadow:0 10px 30px rgba(0,0,0,.06);}
    h1{margin-top:0;font-size:1.8rem;}
    ul{padding-left:20px;}
    a{color:#0e7c66;text-decoration:none;font-weight:600;}
    a:hover{text-decoration:underline;}
  </style>
</head>
<body>
  <div class="card">
    <h1>$title</h1>
    <ul>
      $can_link
      $interactive_link
      <li><a href="logs/">Logs</a></li>
    </ul>
  </div>
</body>
</html>
EOF
}

if [[ "$TARGET" == "interactive_plot" || "$INTERACTIVE_MODE" == "only" ]]; then
  INTERACTIVE_LOG="$LOG_DIR/interactive_plot.log"
  run_interactive_plot "$OUTPUT_DIR" "$INTERACTIVE_LOG"
  exit 0
fi

if [[ "$TARGET" == "udp_kpi" && "$INTERACTIVE_MODE" == "enabled" ]]; then
  KPI_LOG="$LOG_DIR/udp_kpi_server.log"
  INTERACTIVE_LOG="$LOG_DIR/interactive_plot.log"

  "$CTR_CMD" run "${CTR_BIND_OPTS[@]}" "$UDP_KPI_SIMG" zmq "$PORT" >"$KPI_LOG" 2>&1 &
  KPI_PID=$!

  cleanup_udp() {
    if [[ -n "${KPI_PID:-}" ]] && kill -0 "$KPI_PID" >/dev/null 2>&1; then
      kill "$KPI_PID" >/dev/null 2>&1 || true
      wait "$KPI_PID" >/dev/null 2>&1 || true
    fi
  }
  trap cleanup_udp EXIT

  READY=0
  for _ in $(seq 1 90); do
    if ! kill -0 "$KPI_PID" >/dev/null 2>&1; then
      echo "ERROR: UDP KPI server exited early. Check $KPI_LOG" >&2
      tail -n 120 "$KPI_LOG" >&2 || true
      exit 1
    fi
    if grep -q "KPI ZMQ server started on port" "$KPI_LOG"; then
      READY=1
      break
    fi
    sleep 1
  done

  if [[ "$READY" -ne 1 ]]; then
    echo "ERROR: UDP KPI server did not become ready in time. Check $KPI_LOG" >&2
    tail -n 120 "$KPI_LOG" >&2 || true
    exit 1
  fi

  run_interactive_plot "$OUTPUT_DIR" "$INTERACTIVE_LOG"
  exit 0
fi

if [[ "$TARGET" == "can_kpi" && "$INTERACTIVE_MODE" == "enabled" ]]; then
  CAN_OUTPUT_DIR="$OUTPUT_DIR/can_kpi"
  INTERACTIVE_OUTPUT_DIR="$OUTPUT_DIR/interactive_plot"
  CAN_LOG="$LOG_DIR/can_kpi.log"
  INTERACTIVE_LOG="$LOG_DIR/interactive_plot.log"
  mkdir -p "$CAN_OUTPUT_DIR" "$INTERACTIVE_OUTPUT_DIR"

  if [[ "$INPUT_MODE" == "hdf" ]]; then
    "$CTR_CMD" run "${CTR_BIND_OPTS[@]}" "$CAN_KPI_SIMG" hdf "$INPUT_HDF" "$OUTPUT_HDF" "$CAN_OUTPUT_DIR" >"$CAN_LOG" 2>&1
  else
    "$CTR_CMD" run "${CTR_BIND_OPTS[@]}" "$CAN_KPI_SIMG" json "$JSON_PATH" "$CAN_OUTPUT_DIR" >"$CAN_LOG" 2>&1
  fi

  run_interactive_plot "$INTERACTIVE_OUTPUT_DIR" "$INTERACTIVE_LOG"

  CAN_LINK='<li><a href="can_kpi/index.html">CAN KPI output</a></li>'
  INTERACTIVE_LINK='<li><a href="interactive_plot/master_index.html">Interactive Plot output</a></li>'
  write_combined_index "$OUTPUT_DIR/index.html" 'CAN KPI + Interactive Plot' "$CAN_LINK" "$INTERACTIVE_LINK"
  exit 0
fi

echo "ERROR: Unsupported target/mode combination: target=$TARGET interactive_mode=$INTERACTIVE_MODE" >&2
exit 1