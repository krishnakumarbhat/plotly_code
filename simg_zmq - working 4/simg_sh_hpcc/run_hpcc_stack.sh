#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
LOG_DIR="$SCRIPT_DIR/logs"
mkdir -p "$LOG_DIR"

export HPCC_BROKER_HOST="${HPCC_BROKER_HOST:-0.0.0.0}"
export HPCC_BROKER_PORT="${HPCC_BROKER_PORT:-9100}"
export HOST_SIMG_PATH="$SCRIPT_DIR"

MAIN_HTML_IMAGE="$SCRIPT_DIR/main_html.simg"
if [[ ! -f "$MAIN_HTML_IMAGE" ]]; then
    echo "Missing $MAIN_HTML_IMAGE. Build the bundle first." >&2
    exit 1
fi

export HPCC_MAIN_HTML_CMD="singularity run --bind /net:/net --bind /scratch:/scratch --bind /mnt:/mnt $MAIN_HTML_IMAGE"

echo "Starting hpcc_main.py broker on ${HPCC_BROKER_HOST}:${HPCC_BROKER_PORT}"
python3 "$ROOT_DIR/hpcc_main.py" \
    --host "$HPCC_BROKER_HOST" \
    --port "$HPCC_BROKER_PORT" \
    --ui-command "$HPCC_MAIN_HTML_CMD" \
    >> "$LOG_DIR/hpcc_main.log" 2>&1