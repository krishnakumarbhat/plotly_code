#!/usr/bin/env bash
#==============================================================================
# run_all_services_simg.sh - Run the HPC Tools Platform Singularity container
#==============================================================================
# This script runs the all_services Singularity image on HPC clusters
#
# Usage:
#   ./run_all_services_simg.sh [IMAGE_PATH] [PORT]
#   ./run_all_services_simg.sh --help
#
# Options:
#   IMAGE_PATH      Path to .simg file (default: ./all_services.simg)
#   PORT            Port to run on (default: 5001)
#   --debug         Run in debug mode
#   --shell         Open shell instead of running app
#   --help          Show this help message
#
# Supported clusters:
#   - Krakow (10.214.45.45)     - /net/8k3/...
#   - Southfield (10.192.224.131) - /mnt/usmidet/...
#==============================================================================

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Default configuration
IMAGE_PATH="${1:-./all_services.simg}"
PORT="${2:-5001}"
HOST="${HOST:-0.0.0.0}"
WORKERS="${WORKERS:-4}"
DEBUG_MODE=false
SHELL_MODE=false

# Parse optional flags (if first arg starts with --)
while [[ "${1:-}" == --* ]]; do
    case $1 in
        --debug)
            DEBUG_MODE=true
            shift
            ;;
        --shell)
            SHELL_MODE=true
            shift
            ;;
        --help|-h)
            head -20 "$0" | tail -15
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Re-read positional args after flag parsing
IMAGE_PATH="${1:-./all_services.simg}"
PORT="${2:-5001}"

# Detect cluster based on available paths
detect_cluster() {
    if [ -d "/net/8k3" ]; then
        echo "krakow"
    elif [ -d "/mnt/usmidet" ]; then
        echo "southfield"
    else
        echo "unknown"
    fi
}

# Get LLM server URL based on cluster
get_llm_url() {
    local cluster="$1"
    case "$cluster" in
        krakow)
            echo "http://10.214.45.45:8000/generate"
            ;;
        southfield)
            echo "http://10.192.224.131:8000/generate"
            ;;
        *)
            echo "http://localhost:8000/generate"
            ;;
    esac
}

if [ ! -f "$IMAGE_PATH" ]; then
    echo -e "${RED}ERROR: image not found: $IMAGE_PATH${NC}" >&2
    echo "Usage: $0 /path/to/all_services.simg [port]" >&2
    exit 2
fi

RUNTIME_TOOL=""
if command -v apptainer >/dev/null 2>&1; then
    RUNTIME_TOOL="apptainer"
elif command -v singularity >/dev/null 2>&1; then
    RUNTIME_TOOL="singularity"
else
    echo -e "${RED}ERROR: neither 'apptainer' nor 'singularity' is available on PATH.${NC}" >&2
    exit 3
fi

# Detect cluster
CLUSTER=$(detect_cluster)
LLM_URL=$(get_llm_url "$CLUSTER")

# Build bind args only for paths that exist on the host
BIND_ARGS=()
# Krakow paths
if [ -d /net ]; then
    BIND_ARGS+=(--bind /net:/net)
fi
# Southfield paths
if [ -d /mnt ]; then
    BIND_ARGS+=(--bind /mnt:/mnt)
fi
# Common paths
if [ -d /scratch ]; then
    BIND_ARGS+=(--bind /scratch:/scratch)
fi
if [ -d /home ]; then
    BIND_ARGS+=(--bind /home:/home)
fi
if [ -d /tmp ]; then
    BIND_ARGS+=(--bind /tmp:/tmp)
fi

if [ -z "${DATABASE_URL:-}" ]; then
    echo -e "${YELLOW}WARN: DATABASE_URL is not set.${NC}" >&2
    echo "      On Linux, the app defaults to Postgres at localhost:5432." >&2
    echo "      Export DATABASE_URL if you don't have local Postgres running." >&2
fi

echo "============================================================"
echo -e "  ${GREEN}HPC Tools Platform - Singularity Container${NC}"
echo "============================================================"
echo "Cluster:     $CLUSTER"
echo "Runtime:     $RUNTIME_TOOL"
echo "Image:       $IMAGE_PATH"
echo "Port:        $PORT"
echo "Workers:     $WORKERS"
echo "LLM URL:     $LLM_URL"
echo "Bind Paths:  ${BIND_ARGS[*]:-none}"
echo "============================================================"
echo ""
echo -e "${BLUE}Serving: http://127.0.0.1:${PORT}/html${NC}"
echo ""

# Run in different modes
if [ "$SHELL_MODE" = true ]; then
    echo "Opening interactive shell..."
    "$RUNTIME_TOOL" shell --writable-tmpfs "${BIND_ARGS[@]}" "$IMAGE_PATH"
elif [ "$DEBUG_MODE" = true ]; then
    echo "Running in debug mode..."
    env \
        HOST="$HOST" \
        PORT="$PORT" \
        LLM_BASE_URL="$LLM_URL" \
        FLASK_DEBUG=1 \
        "$RUNTIME_TOOL" run --writable-tmpfs "${BIND_ARGS[@]}" "$IMAGE_PATH"
else
    env \
        HOST="$HOST" \
        PORT="$PORT" \
        WORKERS="$WORKERS" \
        LLM_BASE_URL="$LLM_URL" \
        "$RUNTIME_TOOL" run --writable-tmpfs "${BIND_ARGS[@]}" "$IMAGE_PATH"
fi
