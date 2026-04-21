#!/bin/bash
# run_container.sh
# Simple wrapper to run the container image on the cluster and bind the
# project folder into /project. Adjust IMAGE and PROJECT_DIR as needed.

IMAGE="/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/flask_plotly.sif"
PROJECT_DIR="/net/8k3/e0fs01/irods/PLKRA-PROJECTS/RNA-SDV-SRR7/4-Checkout/all_service/project"

if command -v apptainer &>/dev/null; then
  RUNTIME=apptainer
elif command -v singularity &>/dev/null; then
  RUNTIME=singularity
else
  echo "Neither apptainer nor singularity found in PATH" >&2
  exit 2
fi

echo "Using runtime: $RUNTIME"
echo "Image: $IMAGE"

if [ ! -f "$IMAGE" ]; then
  echo "Image not found: $IMAGE" >&2
  exit 3
fi

mkdir -p "$PROJECT_DIR"

$RUNTIME exec --bind "$PROJECT_DIR:/project" "$IMAGE" bash -c 'cd /project && exec python app.py'
