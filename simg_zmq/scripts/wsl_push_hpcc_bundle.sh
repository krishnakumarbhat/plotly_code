#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="${1:-$ROOT_DIR/simg_sh_hpcc}"
HOST="${2:-}"
DEST_DIR="${3:-}"
REMOTE_USER="${4:-${USER:-}}"

usage() {
    cat <<'EOF'
Usage:
  ./scripts/wsl_push_hpcc_bundle.sh [src_dir] <host> <dest_dir> [remote_user]

Example:
  ./scripts/wsl_push_hpcc_bundle.sh ./simg_sh_hpcc 10.192.224.131 /mnt/usmidet/projects/RADARCORE/2-Sim/all_services_3 ouymc2
EOF
}

if [[ -z "$HOST" || -z "$DEST_DIR" ]]; then
    usage >&2
    exit 2
fi

if [[ ! -d "$SRC_DIR" ]]; then
    echo "Source bundle directory not found: $SRC_DIR" >&2
    exit 1
fi

tar -C "$SRC_DIR" -czf - . | ssh -o StrictHostKeyChecking=accept-new "${REMOTE_USER}@${HOST}" "mkdir -p '$DEST_DIR' && tar -xzf - -C '$DEST_DIR'"

echo "Uploaded bundle from $SRC_DIR to ${REMOTE_USER}@${HOST}:$DEST_DIR"