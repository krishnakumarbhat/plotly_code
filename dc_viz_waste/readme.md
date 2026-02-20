# DC Viz — README

Summary
- DC Viz displays radar visualization videos and associated logs in a local static viewer and supports an optional VLM-based automatic description generator.
- Key files:
  - `main.py` — CLI entrypoint (modes: `--serve`, `--vlm`, `--force`)
  - `vlm.py` — Vision-Language Model integration and sequential video processing
  - `html_build.py` — Static HTML renderer and UI assets
  - `cluster_connect.py` — Cluster download logic + progress tracking
  - `models/` — local model files (e.g. `Qwen3-VL-2B-Instruct`)
  - `db/` — input data (`db/html`, `db/video`)
  - `out/` — generated viewer (`out/viewer.html`) and static assets
  - `dist/` — built executables (PyInstaller)

Repository layout (important paths)
- <repo root>
  - main.py
  - vlm.py
  - html_build.py
  - cluster_connect.py
  - models/Qwen3-VL-2B-Instruct/ (local safetensors + config.json)
  - db/html/ (per-video HTML artifacts)
  - db/video/ (video files; generated text stored in `db/video/log_txt/`)
  - out/ (viewer.html and static files)
  - dist/ (optional: built exe)

Design notes
- Paths are resolved relative to the script/EXE location (BASE_DIR) wherever possible. This prevents 404s when running the EXE from outside the project — but you must still point the EXE to the real `db/` and `out/` folders if they are not next to the EXE.
- The VLM mode (`--vlm`) processes videos sequentially and only sends videos that lack a `_web.txt` file (unless `--force` is specified).
- Video description files are saved under `<video_root>/log_txt/<basename>_web.txt`. If `<basename>` already ends with `_web`, it’s normalized.

Usage

1) Serve mode (interactive viewer)
- Defaults (relative to executable/script base dir):
  - html_root: `<BASE_DIR>/db/html`
  - video_root: `<BASE_DIR>/db/video`
  - output_html: `<BASE_DIR>/out/viewer.html`
- Examples (Windows PowerShell):
  - Use defaults (viewer reads local db and out next to exe/script):
    .\log_viewer.exe --serve
  - Provide specific paths (absolute paths recommended when exe is outside project):
    .\log_viewer.exe --serve "C:\Users\you\Desktop\dc_viz\db\html" "C:\Users\you\Desktop\dc_viz\db\video" "C:\Users\you\Desktop\dc_viz\out\viewer.html"
  - If you omit the third arg, the app uses `<BASE_DIR>/out/viewer.html`.

Notes for avoiding 404 when running EXE outside repo:
- The EXE does not carry your `db/` or `out/` folders — point it to the real locations with absolute paths.
- main.py uses `OUT_DIR` / `BASE_DIR` to serve static files; if you keep defaults, ensure `out/viewer.html` lives next to the EXE or rebuild with the correct embedded path.

2) VLM mode (`--vlm`) — sequential video description generation
- Purpose: generate a single-line description per video using a Vision-Language Model.
- Behavior:
  - Scans `video_root` for video files (.mp4, .avi, .mov, .mkv)
  - For each video, if `<video_root>/log_txt/<basename>_web.txt` exists and `--force` is not provided → skip
  - Otherwise, load or fallback to HF model and generate description, then save to `_web.txt`
  - Processes videos one at a time (sequential) to keep memory and CPU usage low
- CLI:
  - Default folder:
    python main.py --vlm
  - Specify folder:
    .\log_viewer.exe --vlm "C:\path\to\videos"
  - Force regenerate:
    .\log_viewer.exe --vlm "C:\path\to\videos" --force

3) Offline model usage
- If a local model exists at `models/Qwen3-VL-2B-Instruct/`, `vlm.py` will attempt to load it offline.
- If local model is missing, the code falls back to HF remote model (requires internet).

VLM file naming
- Input video: `foo_web.mp4` or `foo.mp4`
- Output text: `<video_root>/log_txt/foo_web.txt` (strip or append `_web` consistently)

Building the EXE (PyInstaller)
- Typical build command used during development:
  pyinstaller --clean --onefile --add-data "out;out" --add-data "models;models" main.py
- If you move the EXE outside the repo, run it with absolute data paths or include the `db/` and `out/` folders alongside the exe.

Quantization & model-size reduction (quick reference)
- Goal: reduce `model.safetensors` size to make it easier to store or distribute (note: GitHub file limit ~100 MB).
- Practical options:
  1) GPTQ 4-bit quantization (AutoGPTQ): largest size reduction, requires conversion tool (often GPU), specialized runtime to load quantized model.
  2) 8-bit runtime (bitsandbytes): reduces runtime memory but not always on-disk size; good for inference if you keep the large source model elsewhere.
  3) Convert to GGUF / ggml (llama.cpp style): produces compact CPU binary; converter requirements vary by model architecture.

- Short local script examples are available in the repo notes (or re-run the provided scripts to convert a locally present model). Conversion usually requires:
  - A machine with a GPU (for GPTQ conversion)
  - Installed tools: `auto-gptq`, `bitsandbytes`, or a ggml converter
  - Careful testing — quantization can change model quality.

Presentation checklist (sequence)
1. Title & goal (30s) — show CLI and modes.
2. Repo layout & responsibilities of main files (1 min) — show architecture.dot.
3. Demo: Serve mode (2–3 min)
   - Start server with defaults.
   - Show viewer at http://localhost:5000.
   - Demonstrate edit/save behavior (serve mode supports polling endpoints to refresh content).
4. Demo: VLM mode (2–4 min)
   - Show `db/video` before.
   - Run `--vlm` (or `--vlm --force`) and show console progress lines per-video.
   - Show generated text files in `db/video/log_txt/`.
5. Troubleshooting & tips (1 min)
   - EXE 404s → pass absolute `output_html` or ensure OUT_DIR is next to exe.
   - Large models → quantize or use adapters/LoRA instead of committing big safetensors to git.
6. Next steps (30s): add progress API integration for long-running VLM runs, optional GPU support, or adapter-only distribution.

Troubleshooting (common issues)
- 404 when serving viewer.html:
  - Ensure `send_from_directory()` in `main.py` uses absolute `OUT_DIR`.
  - When running exe outside the project, pass full absolute paths for html_root/video_root/output_html.
- VLM slow on CPU:
  - Use fewer frames per video (already sampled down to ~4 frames).
  - Use a GPU to load and run model or convert to a CPU-friendly format (GGUF).
- Model fails to load:
  - Confirm `models/<model-name>/` contains required `config.json` and safetensors.
  - If offline, set `local_files_only=True` and supply local model files.

Files added by this README
- `architecture.dot` — graphviz DOT file to visualize module relationships (use `dot -Tpng architecture.dot -o architecture.png`)

Contact & next actions
- If you want, I can:
  - Produce PowerPoint-ready slide text for the demo steps.
  - Add a small script in `scripts/` to automate building a compact quantized model (requires GPU environment).
  - Patch `main.py` to always normalize CLI args to absolute paths (I can apply the code changes and a small test).

```// filepath: c:\Users\ouymc2\Desktop\dc_viz\README.md
# DC Viz — README

Summary
- DC Viz displays radar visualization videos and associated logs in a local static viewer and supports an optional VLM-based automatic description generator.
- Key files:
  - `main.py` — CLI entrypoint (modes: `--serve`, `--vlm`, `--force`)
  - `vlm.py` — Vision-Language Model integration and sequential video processing
  - `html_build.py` — Static HTML renderer and UI assets
  - `cluster_connect.py` — Cluster download logic + progress tracking
  - `models/` — local model files (e.g. `Qwen3-VL-2B-Instruct`)
  - `db/` — input data (`db/html`, `db/video`)
  - `out/` — generated viewer (`out/viewer.html`) and static assets
  - `dist/` — built executables (PyInstaller)

Repository layout (important paths)
- <repo root>
  - main.py
  - vlm.py
  - html_build.py
  - cluster_connect.py
  - models/Qwen3-VL-2B-Instruct/ (local safetensors + config.json)
  - db/html/ (per-video HTML artifacts)
  - db/video/ (video files; generated text stored in `db/video/log_txt/`)
  - out/ (viewer.html and static files)
  - dist/ (optional: built exe)

Design notes
- Paths are resolved relative to the script/EXE location (BASE_DIR) wherever possible. This prevents 404s when running the EXE from outside the project — but you must still point the EXE to the real `db/` and `out/` folders if they are not next to the EXE.
- The VLM mode (`--vlm`) processes videos sequentially and only sends videos that lack a `_web.txt` file (unless `--force` is specified).
- Video description files are saved under `<video_root>/log_txt/<basename>_web.txt`. If `<basename>` already ends with `_web`, it’s normalized.

Usage

1) Serve mode (interactive viewer)
- Defaults (relative to executable/script base dir):
  - html_root: `<BASE_DIR>/db/html`
  - video_root: `<BASE_DIR>/db/video`
  - output_html: `<BASE_DIR>/out/viewer.html`
- Examples (Windows PowerShell):
  - Use defaults (viewer reads local db and out next to exe/script):
    .\log_viewer.exe --serve
  - Provide specific paths (absolute paths recommended when exe is outside project):
    .\log_viewer.exe --serve "C:\Users\you\Desktop\dc_viz\db\html" "C:\Users\you\Desktop\dc_viz\db\video" "C:\Users\you\Desktop\dc_viz\out\viewer.html"
  - If you omit the third arg, the app uses `<BASE_DIR>/out/viewer.html`.

Notes for avoiding 404 when running EXE outside repo:
- The EXE does not carry your `db/` or `out/` folders — point it to the real locations with absolute paths.
- main.py uses `OUT_DIR` / `BASE_DIR` to serve static files; if you keep defaults, ensure `out/viewer.html` lives next to the EXE or rebuild with the correct embedded path.

2) VLM mode (`--vlm`) — sequential video description generation
- Purpose: generate a single-line description per video using a Vision-Language Model.
- Behavior:
  - Scans `video_root` for video files (.mp4, .avi, .mov, .mkv)
  - For each video, if `<video_root>/log_txt/<basename>_web.txt` exists and `--force` is not provided → skip
  - Otherwise, load or fallback to HF model and generate description, then save to `_web.txt`
  - Processes videos one at a time (sequential) to keep memory and CPU usage low
- CLI:
  - Default folder:
    python main.py --vlm
  - Specify folder:
    .\log_viewer.exe --vlm "C:\path\to\videos"
  - Force regenerate:
    .\log_viewer.exe --vlm "C:\path\to\videos" --force

3) Offline model usage
- If a local model exists at `models/Qwen3-VL-2B-Instruct/`, `vlm.py` will attempt to load it offline.
- If local model is missing, the code falls back to HF remote model (requires internet).

VLM file naming
- Input video: `foo_web.mp4` or `foo.mp4`
- Output text: `<video_root>/log_txt/foo_web.txt` (strip or append `_web` consistently)

Building the EXE (PyInstaller)
- Typical build command used during development:
  pyinstaller --clean --onefile --add-data "out;out" --add-data "models;models" main.py
- If you move the EXE outside the repo, run it with absolute data paths or include the `db/` and `out/` folders alongside the exe.

Quantization & model-size reduction (quick reference)
- Goal: reduce `model.safetensors` size to make it easier to store or distribute (note: GitHub file limit ~100 MB).
- Practical options:
  1) GPTQ 4-bit quantization (AutoGPTQ): largest size reduction, requires conversion tool (often GPU), specialized runtime to load quantized model.
  2) 8-bit runtime (bitsandbytes): reduces runtime memory but not always on-disk size; good for inference if you keep the large source model elsewhere.
  3) Convert to GGUF / ggml (llama.cpp style): produces compact CPU binary; converter requirements vary by model architecture.

- Short local script examples are available in the repo notes (or re-run the provided scripts to convert a locally present model). Conversion usually requires:
  - A machine with a GPU (for GPTQ conversion)
  - Installed tools: `auto-gptq`, `bitsandbytes`, or a ggml converter
  - Careful testing — quantization can change model quality.

Presentation checklist (sequence)
1. Title & goal (30s) — show CLI and modes.
2. Repo layout & responsibilities of main files (1 min) — show architecture.dot.
3. Demo: Serve mode (2–3 min)
   - Start server with defaults.
   - Show viewer at http://localhost:5000.
   - Demonstrate edit/save behavior (serve mode supports polling endpoints to refresh content).
4. Demo: VLM mode (2–4 min)
   - Show `db/video` before.
   - Run `--vlm` (or `--vlm --force`) and show console progress lines per-video.
   - Show generated text files in `db/video/log_txt/`.
5. Troubleshooting & tips (1 min)
   - EXE 404s → pass absolute `output_html` or ensure OUT_DIR is next to exe.
   - Large models → quantize or use adapters/LoRA instead of committing big safetensors to git.
6. Next steps (30s): add progress API integration for long-running VLM runs, optional GPU support, or adapter-only distribution.

Troubleshooting (common issues)
- 404 when serving viewer.html:
  - Ensure `send_from_directory()` in `main.py` uses absolute `OUT_DIR`.
  - When running exe outside the project, pass full absolute paths for html_root/video_root/output_html.
- VLM slow on CPU:
  - Use fewer frames per video (already sampled down to ~4 frames).
  - Use a GPU to load and run model or convert to a CPU-friendly format (GGUF).
- Model fails to load:
  - Confirm `models/<model-name>/` contains required `config.json` and safetensors.
  - If offline, set `local_files_only=True` and supply local model files.

Files added by this README
- `architecture.dot` — graphviz DOT file to visualize module relationships (use `dot -Tpng architecture.dot -o architecture.png`)

Contact & next actions
- If you want, I can:
  - Produce PowerPoint-ready slide text for the demo steps.
  - Add a small script in `scripts/` to automate building a compact quantized model (requires GPU environment).
  - Patch `main.py` to always normalize CLI args to absolute paths (I can apply the code changes and a small test).
