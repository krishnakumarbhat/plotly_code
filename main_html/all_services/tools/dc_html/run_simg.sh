#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
WORKDIR=$(pwd)

if command -v apptainer >/dev/null 2>&1; then
  RUNTIME="apptainer"
elif command -v singularity >/dev/null 2>&1; then
  RUNTIME="singularity"
else
  echo "Neither apptainer nor singularity is installed."
  exit 1
fi

usage() {
  echo "Usage:"
  echo "  $0 <HTMLConfig.xml> <Inputs.json> <output_dir>"
  echo "  $0 <IMAGE.simg> <HTMLConfig.xml> <Inputs.json> <output_dir>"
}

if [ "$#" -eq 3 ]; then
  IMG="ResimHTMLReport.simg"
  CONFIG="$1"
  INPUT_JSON="$2"
  OUT_DIR="$3"
elif [ "$#" -eq 4 ]; then
  IMG="$1"
  CONFIG="$2"
  INPUT_JSON="$3"
  OUT_DIR="$4"
else
  usage
  exit 1
fi

resolve_file() {
  input="$1"
  if [ "${input#/}" != "$input" ]; then
    candidate="$input"
    [ -f "$candidate" ] && { printf '%s' "$candidate"; return 0; }
    return 1
  fi

  candidate="$WORKDIR/$input"
  [ -f "$candidate" ] && { printf '%s' "$candidate"; return 0; }

  candidate="$SCRIPT_DIR/$input"
  [ -f "$candidate" ] && { printf '%s' "$candidate"; return 0; }

  return 1
}

resolve_dir_target() {
  input="$1"
  if [ "${input#/}" != "$input" ]; then
    printf '%s' "$input"
  else
    printf '%s' "$WORKDIR/$input"
  fi
}

if ! IMG_ABS=$(resolve_file "$IMG"); then
  echo "Image not found: $IMG"
  usage
  exit 1
fi

if ! CONFIG_ABS=$(resolve_file "$CONFIG"); then
  echo "Config not found: $CONFIG"
  exit 1
fi

if ! INPUT_JSON_ABS=$(resolve_file "$INPUT_JSON"); then
  echo "Inputs JSON not found: $INPUT_JSON"
  exit 1
fi

OUT_DIR_ABS=$(resolve_dir_target "$OUT_DIR")
mkdir -p "$OUT_DIR_ABS"

TMP_JSON=$(mktemp "${TMPDIR:-/tmp}/resim_inputs_abs.XXXXXX.json")
trap 'rm -f "$TMP_JSON"' EXIT INT TERM

# Create temp JSON with absolute InputHDF/OutputHDF paths resolved relative to JSON dir.
python3 - "$INPUT_JSON_ABS" "$TMP_JSON" <<'PY'
import json
import os
import sys

src = sys.argv[1]
dst = sys.argv[2]
base = os.path.abspath(os.path.dirname(src))

with open(src, "r", encoding="utf-8") as f:
    data = json.load(f)

def to_abs(p):
    if not isinstance(p, str):
        return p
    p = p.strip()
    if not p:
        return p
    return os.path.abspath(p) if os.path.isabs(p) else os.path.abspath(os.path.join(base, p))

for key in ("InputHDF", "OutputHDF"):
    v = data.get(key)
    if isinstance(v, list):
        data[key] = [to_abs(x) for x in v]

with open(dst, "w", encoding="utf-8") as f:
    json.dump(data, f, indent=2)
PY

# Build bind list as comma-separated host:container pairs.
BIND_SPEC=""
append_bind() {
  d="$1"
  [ -d "$d" ] || return 0
  pair="$d:$d"
  case ",$BIND_SPEC," in
    *",$pair,"*) return 0 ;;
  esac
  if [ -n "$BIND_SPEC" ]; then
    BIND_SPEC="$BIND_SPEC,$pair"
  else
    BIND_SPEC="$pair"
  fi
}

append_bind "$WORKDIR"
append_bind "$SCRIPT_DIR"
append_bind "$(dirname "$IMG_ABS")"
append_bind "$(dirname "$CONFIG_ABS")"
append_bind "$(dirname "$INPUT_JSON_ABS")"
append_bind "$OUT_DIR_ABS"
append_bind "$(dirname "$TMP_JSON")"

# Add all HDF parent dirs from normalized temp JSON.
HDF_DIRS=$(python3 - "$TMP_JSON" <<'PY'
import json
import os
import sys

path = sys.argv[1]
with open(path, "r", encoding="utf-8") as f:
    cfg = json.load(f)

seen = set()
for key in ("InputHDF", "OutputHDF"):
    vals = cfg.get(key, [])
    if not isinstance(vals, list):
        continue
    for p in vals:
        if not isinstance(p, str) or not p.strip():
            continue
        d = os.path.abspath(os.path.dirname(p))
        if d and d not in seen:
            seen.add(d)
            print(d)
PY
)

if [ -n "$HDF_DIRS" ]; then
  OLD_IFS=$IFS
  IFS='\n'
  for d in $HDF_DIRS; do
    [ -n "$d" ] && append_bind "$d"
  done
  IFS=$OLD_IFS
fi

# Ensure common cluster roots are available if present.
for root in /net /gpfs /scratch /home /tmp; do
  append_bind "$root"
done

echo "Runtime     : $RUNTIME"
echo "Image       : $IMG_ABS"
echo "Config      : $CONFIG_ABS"
echo "Inputs      : $INPUT_JSON_ABS"
echo "Inputs(temp): $TMP_JSON"
echo "Output dir  : $OUT_DIR_ABS"

exec "$RUNTIME" run \
  --bind "$BIND_SPEC" \
  --pwd "$WORKDIR" \
  "$IMG_ABS" \
  "$CONFIG_ABS" \
  "$TMP_JSON" \
  "$OUT_DIR_ABS"
