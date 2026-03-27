#!/usr/bin/env bash
set -euo pipefail

# Unified launcher:
# - MUDP    -> InteractivePlot + UDP_KPI (ZMQ)
# - BORDNET -> CAN_KPI only (direct JSON batch, no ZMQ)

usage() {
  cat <<'EOF'
Usage:
  main.sh <config.xml> <inputs.json> <output_dir> \
    [--interactive-simg /path/interactiveplot.simg] \
    [--udp-kpi-simg /path/kpi.simg] \
    [--can-kpi-simg /path/can_kpi.simg] \
    [--port 5560] \
    [--log-dir /path/logs] \
    [--slurm-account RNA-SDV-SRR7] \
    [--slurm-partition highPrio] \
    [--slurm-mem 72G] \
    [--slurm-time 18:00:00] \
    [--no-srun] \
    [--bind /path1:/path1] [--bind /path2:/path2] ...

Behavior:
  HDF_SOURCE_SELECTION=MUDP    -> runs InteractivePlot + UDP_KPI over ZMQ
  HDF_SOURCE_SELECTION=BORDNET -> runs CAN_KPI directly from JSON pairs
EOF
}

require_file() {
  local p="$1"
  local name="$2"
  if [[ ! -f "$p" ]]; then
    echo "ERROR: $name not found: $p" >&2
    exit 1
  fi
}

CONFIG_XML="${1:-}"
INPUT_JSON="${2:-}"
OUTPUT_DIR="${3:-}"
ORIGINAL_ARGS=("$@")

if [[ -z "$CONFIG_XML" || -z "$INPUT_JSON" || -z "$OUTPUT_DIR" ]]; then
  usage
  exit 1
fi
shift 3

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INTERACTIVE_SIMG="$SCRIPT_DIR/interactiveplot.simg"
UDP_KPI_SIMG="$SCRIPT_DIR/kpi.simg"
CAN_KPI_SIMG="$SCRIPT_DIR/can_kpi.simg"
PORT="5560"
LOG_DIR=""
BIND_TARGETS=()
AUTO_SRUN=1
SLURM_ACCOUNT="RNA-SDV-SRR7"
SLURM_PARTITION="highPrio"
SLURM_MEM="72G"
SLURM_TIME="18:00:00"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --interactive-simg) INTERACTIVE_SIMG="$2"; shift 2 ;;
    --udp-kpi-simg) UDP_KPI_SIMG="$2"; shift 2 ;;
    --can-kpi-simg) CAN_KPI_SIMG="$2"; shift 2 ;;
    --port) PORT="$2"; shift 2 ;;
    --log-dir) LOG_DIR="$2"; shift 2 ;;
    --slurm-account) SLURM_ACCOUNT="$2"; shift 2 ;;
    --slurm-partition) SLURM_PARTITION="$2"; shift 2 ;;
    --slurm-mem) SLURM_MEM="$2"; shift 2 ;;
    --slurm-time) SLURM_TIME="$2"; shift 2 ;;
    --no-srun) AUTO_SRUN=0; shift 1 ;;
    --bind) BIND_TARGETS+=("$2"); shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "ERROR: Unknown argument: $1" >&2; usage; exit 1 ;;
  esac
done

if [[ "$AUTO_SRUN" -eq 1 && -z "${SLURM_JOB_ID:-}" && -z "${MAIN_SH_SKIP_SRUN:-}" ]]; then
  if ! command -v srun >/dev/null 2>&1; then
    if command -v module >/dev/null 2>&1; then
      module load slurm >/dev/null 2>&1 || true
    elif [[ -f /etc/profile.d/modules.sh ]]; then
      # shellcheck disable=SC1091
      source /etc/profile.d/modules.sh >/dev/null 2>&1 || true
      if command -v module >/dev/null 2>&1; then
        module load slurm >/dev/null 2>&1 || true
      fi
    fi
  fi

  if command -v srun >/dev/null 2>&1; then
    echo "[SLURM] No active allocation detected. Re-launching via srun..."
    echo "[SLURM] account=$SLURM_ACCOUNT partition=$SLURM_PARTITION mem=$SLURM_MEM time=$SLURM_TIME"
    export MAIN_SH_SKIP_SRUN=1
    exec srun -A "$SLURM_ACCOUNT" --partition="$SLURM_PARTITION" --mem="$SLURM_MEM" --time="$SLURM_TIME" \
      bash "$SCRIPT_DIR/main.sh" "${ORIGINAL_ARGS[@]}"
  else
    echo "[SLURM] WARNING: srun not found; continuing without auto-allocation." >&2
  fi
fi

require_file "$CONFIG_XML" "Config XML"
require_file "$INPUT_JSON" "Input JSON"

CONFIG_XML="$(python3 -c 'import os,sys; print(os.path.abspath(sys.argv[1]))' "$CONFIG_XML")"
INPUT_JSON="$(python3 -c 'import os,sys; print(os.path.abspath(sys.argv[1]))' "$INPUT_JSON")"
OUTPUT_DIR="$(python3 -c 'import os,sys; print(os.path.abspath(sys.argv[1]))' "$OUTPUT_DIR")"
INTERACTIVE_SIMG="$(python3 -c 'import os,sys; print(os.path.abspath(sys.argv[1]))' "$INTERACTIVE_SIMG")"
UDP_KPI_SIMG="$(python3 -c 'import os,sys; print(os.path.abspath(sys.argv[1]))' "$UDP_KPI_SIMG")"
CAN_KPI_SIMG="$(python3 -c 'import os,sys; print(os.path.abspath(sys.argv[1]))' "$CAN_KPI_SIMG")"

mkdir -p "$OUTPUT_DIR"
umask 022

if ! command -v python3 >/dev/null 2>&1; then
  echo "ERROR: python3 is required" >&2
  exit 1
fi

try_load_module() {
  local module_name="$1"
  if command -v module >/dev/null 2>&1; then
    module load "$module_name" >/dev/null 2>&1 || true
    return 0
  fi

  # In non-interactive shells, module may not be initialized.
  if [[ -f /etc/profile.d/modules.sh ]]; then
    # shellcheck disable=SC1091
    source /etc/profile.d/modules.sh >/dev/null 2>&1 || true
  fi
  if command -v module >/dev/null 2>&1; then
    module load "$module_name" >/dev/null 2>&1 || true
    return 0
  fi

  # Fallback for environments exposing modulecmd only.
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

  # Try common HPC module names.
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
  echo "Try one of the following on HPCC before running main.sh:" >&2
  echo "  module load apptainer" >&2
  echo "  module load singularity" >&2
  echo "Also ensure you are on a compute node where container runtime is enabled." >&2
  exit 1
fi

CTR_BIND_OPTS=()
for target in "${BIND_TARGETS[@]}"; do
  CTR_BIND_OPTS+=("-B" "$target")
done

SOURCE_SELECTION=$(python3 - "$CONFIG_XML" <<'PY'
import sys
import xml.etree.ElementTree as ET

cfg = sys.argv[1]
root = ET.parse(cfg).getroot()
text = (root.findtext("HDF_SOURCE_SELECTION") or "").strip().upper()
print(text)
PY
)

if [[ "$SOURCE_SELECTION" == "MUDP" ]]; then
  require_file "$INTERACTIVE_SIMG" "InteractivePlot SIMG"
  require_file "$UDP_KPI_SIMG" "UDP KPI SIMG"

  if [[ -n "$LOG_DIR" ]]; then
    :
  else
    LOG_DIR="$OUTPUT_DIR/simg_logs"
  fi
  mkdir -p "$LOG_DIR"

  KPI_LOG="$LOG_DIR/kpi_server.log"
  INT_LOG="$LOG_DIR/interactiveplot.log"

  echo "[MUDP 1/3] Starting UDP KPI ZMQ server on port $PORT"
  "$CTR_CMD" run "${CTR_BIND_OPTS[@]}" "$UDP_KPI_SIMG" zmq "$PORT" >"$KPI_LOG" 2>&1 &
  KPI_PID=$!

  cleanup_mudp() {
    if [[ -n "${KPI_PID:-}" ]] && kill -0 "$KPI_PID" >/dev/null 2>&1; then
      kill "$KPI_PID" >/dev/null 2>&1 || true
      wait "$KPI_PID" >/dev/null 2>&1 || true
    fi
  }
  trap cleanup_mudp EXIT

  READY=0
  for _ in $(seq 1 90); do
    if ! kill -0 "$KPI_PID" >/dev/null 2>&1; then
      echo "ERROR: KPI server exited early. Check $KPI_LOG" >&2
      tail -n 80 "$KPI_LOG" >&2 || true
      exit 1
    fi
    if grep -q "KPI ZMQ server started on port" "$KPI_LOG"; then
      READY=1
      break
    fi
    sleep 1
  done

  if [[ "$READY" -ne 1 ]]; then
    echo "ERROR: KPI server did not become ready in time. Check $KPI_LOG" >&2
    tail -n 80 "$KPI_LOG" >&2 || true
    exit 1
  fi

  echo "[MUDP 2/3] Running InteractivePlot with ZMQ target 127.0.0.1:$PORT"
  "$CTR_CMD" run "${CTR_BIND_OPTS[@]}" \
    --env "KPI_SERVER_HOST=127.0.0.1,KPI_SERVER_PORT=$PORT" \
    "$INTERACTIVE_SIMG" "$CONFIG_XML" "$INPUT_JSON" "$OUTPUT_DIR" >"$INT_LOG" 2>&1

  if ! grep -q "Report Generation Completed" "$INT_LOG"; then
    echo "ERROR: InteractivePlot did not report completion. Check $INT_LOG" >&2
    tail -n 120 "$INT_LOG" >&2 || true
    exit 1
  fi

  MASTER_INDEX="$OUTPUT_DIR/master_index.html"
  if [[ ! -f "$MASTER_INDEX" ]]; then
    echo "ERROR: Expected output file not found: $MASTER_INDEX" >&2
    tail -n 120 "$INT_LOG" >&2 || true
    exit 1
  fi

  HTML_COUNT=$(find "$OUTPUT_DIR" -type f -name "*.html" | wc -l | tr -d ' ')
  if [[ "${HTML_COUNT:-0}" -lt 2 ]]; then
    echo "ERROR: HTML output seems empty (count=$HTML_COUNT). Check $INT_LOG" >&2
    tail -n 120 "$INT_LOG" >&2 || true
    exit 1
  fi

  echo "[MUDP 3/3] Completed"
  echo "Interactive output: $OUTPUT_DIR"
  echo "Master index:       $MASTER_INDEX"
  echo "KPI log:            $KPI_LOG"
  echo "Interactive log:    $INT_LOG"
  echo "HTML count:         $HTML_COUNT"
  exit 0
fi

if [[ "$SOURCE_SELECTION" == "BORDNET" ]]; then
  require_file "$CAN_KPI_SIMG" "CAN KPI SIMG"

  if [[ -z "$LOG_DIR" ]]; then
    LOG_DIR="$OUTPUT_DIR/simg_logs"
  fi
  mkdir -p "$LOG_DIR"

  NORMALIZED_JSON="$LOG_DIR/can_kpi_input.normalized.json"
  CAN_HTML_OUT="$OUTPUT_DIR/html/CAN_KPI"
  CAN_LOG="$LOG_DIR/can_kpi.log"

  python3 - "$INPUT_JSON" "$NORMALIZED_JSON" <<'PY'
import json
import os
import sys

inp, outp = sys.argv[1], sys.argv[2]

def normalize_path(p: str) -> str:
  if not isinstance(p, str):
    return p
  s = p.strip().replace('\\\\', '/')
  if len(s) >= 3 and s[1] == ':' and s[2] == '/':
    drv = s[0].lower()
    return f"/mnt/{drv}/{s[3:]}"
  return s

with open(inp, 'r', encoding='utf-8') as f:
  data = json.load(f)

ins = data.get('INPUT_HDF', [])
outs = data.get('OUTPUT_HDF', [])
if not isinstance(ins, list) or not isinstance(outs, list):
  raise SystemExit('INPUT_HDF and OUTPUT_HDF must be arrays')
if len(ins) != len(outs):
  raise SystemExit(f'INPUT_HDF/OUTPUT_HDF length mismatch: {len(ins)} != {len(outs)}')

norm = {
  'INPUT_HDF': [normalize_path(x) for x in ins],
  'OUTPUT_HDF': [normalize_path(x) for x in outs],
}

missing = []
for k in ('INPUT_HDF', 'OUTPUT_HDF'):
  for p in norm[k]:
    if not isinstance(p, str) or not p.strip() or not os.path.exists(p):
      missing.append((k, p))

if missing:
  print('Missing HDF paths after normalization:', file=sys.stderr)
  for k, p in missing[:20]:
    print(f'  - {k}: {p}', file=sys.stderr)
  if len(missing) > 20:
    print(f'  ... and {len(missing)-20} more', file=sys.stderr)
  raise SystemExit(2)

with open(outp, 'w', encoding='utf-8') as f:
  json.dump(norm, f, indent=2)

print(f'Normalized pairs: {len(norm["INPUT_HDF"])}')
print(f'Wrote: {outp}')
PY

  mkdir -p "$CAN_HTML_OUT"

  echo "[BORDNET] Running CAN KPI directly (no InteractivePlot, no ZMQ)"
  "$CTR_CMD" run "${CTR_BIND_OPTS[@]}" "$CAN_KPI_SIMG" json "$NORMALIZED_JSON" "$CAN_HTML_OUT" >"$CAN_LOG" 2>&1

  if [[ ! -f "$CAN_HTML_OUT/index.html" ]]; then
    echo "ERROR: CAN KPI index not generated: $CAN_HTML_OUT/index.html" >&2
    tail -n 120 "$CAN_LOG" >&2 || true
    exit 1
  fi

  HTML_COUNT=$(find "$CAN_HTML_OUT" -type f -name "*.html" | wc -l | tr -d ' ')
  if [[ "${HTML_COUNT:-0}" -lt 2 ]]; then
    echo "ERROR: CAN KPI HTML output seems incomplete (count=$HTML_COUNT)" >&2
    tail -n 120 "$CAN_LOG" >&2 || true
    exit 1
  fi

  echo "[BORDNET] Completed"
  echo "CAN output: $CAN_HTML_OUT"
  echo "CAN log:    $CAN_LOG"
  echo "HTML count: $HTML_COUNT"
  exit 0
fi

echo "ERROR: Unsupported HDF_SOURCE_SELECTION in XML: '$SOURCE_SELECTION'" >&2
echo "Expected one of: MUDP, BORDNET" >&2
exit 1
