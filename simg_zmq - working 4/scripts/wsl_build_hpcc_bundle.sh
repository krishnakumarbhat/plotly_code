#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="$ROOT_DIR/simg_sh_hpcc"

if ! command -v apptainer >/dev/null 2>&1 && ! command -v singularity >/dev/null 2>&1; then
    echo "Apptainer/Singularity is required in WSL to build the HPCC bundle." >&2
    exit 1
fi

BUILDER="apptainer"
if ! command -v "$BUILDER" >/dev/null 2>&1; then
    BUILDER="singularity"
fi

mkdir -p "$OUT_DIR"

build_image() {
    local target_name="$1"
    local definition_file="$2"

    echo "[build] $target_name <- $definition_file"
    "$BUILDER" build --force --fakeroot "$OUT_DIR/$target_name" "$definition_file"
}

build_image "main_html.simg" "$ROOT_DIR/Singularity.def"
build_image "can_kpi.simg" "$ROOT_DIR/KPI/can_kpi/can_singularity_KPI.def"
build_image "udp_kpi.simg" "$ROOT_DIR/KPI/UDP_KPI/Singularity_KPI.def"
build_image "interactiveplot.simg" "$ROOT_DIR/KPI/intplot_kpi/singularity_interactiveplot.def"
build_image "rag.simg" "$ROOT_DIR/rag/Singularity_RAG.def"

echo "[done] HPCC bundle written to $OUT_DIR"