import os
import sys
import json
from typing import Any, Dict, List, Optional


def extract_key(name: str) -> Optional[str]:
    """Extract mapping key from a file or directory name.

    Strategy: split by '_' and join the last two tokens, e.g.
    - 'CCA_9010_1_asdf'      -> '1_asdf'
    - 'SADHFKJASDF_1_asdf'   -> '1_asdf'
    - 'dagfjsdhaf_3_ert.mp4' -> '3_ert'
    """

    base = os.path.splitext(name)[0]
    parts = base.split("_")
    if len(parts) < 2:
        return None
    return "_".join(parts[-2:])


def scan_html_root(html_root: str, output_dir: str) -> Dict[str, List[str]]:
    """Return mapping: key -> list of HTML file paths (relative to output_dir)."""

    html_root = os.path.abspath(html_root)
    mapping: Dict[str, List[str]] = {}

    if not os.path.isdir(html_root):
        raise FileNotFoundError(f"HTML root not found: {html_root}")

    for entry in os.scandir(html_root):
        if not entry.is_dir():
            # ignore loose files like master_index.html
            continue

        key = extract_key(entry.name)
        if not key:
            continue

        html_files: List[str] = []
        for root, _dirs, files in os.walk(entry.path):
            for fname in files:
                ext = os.path.splitext(fname)[1].lower()
                if ext not in {".html", ".htm"}:
                    continue
                full_path = os.path.join(root, fname)
                rel_path = os.path.relpath(full_path, output_dir)
                html_files.append(rel_path.replace("\\", "/"))

        if html_files:
            html_files.sort()
            mapping[key] = html_files

    return mapping


def scan_video_root(video_root: str, output_dir: str) -> Dict[str, str]:
    """Return mapping: key -> video path (relative to output_dir)."""

    video_root = os.path.abspath(video_root)
    mapping: Dict[str, str] = {}

    if not os.path.isdir(video_root):
        raise FileNotFoundError(f"Video root not found: {video_root}")

    exts = {".mp4", ".avi", ".mov", ".mkv"}

    for entry in os.scandir(video_root):
        if not entry.is_file():
            continue

        ext = os.path.splitext(entry.name)[1].lower()
        if ext not in exts:
            continue

        key = extract_key(entry.name)
        if not key:
            continue

        rel_path = os.path.relpath(entry.path, output_dir)
        mapping[key] = rel_path.replace("\\", "/")

    return mapping


def build_viewer_html(output_html: str, mapping: Dict[str, Dict[str, Any]]) -> None:
    """Write the viewer HTML file using the provided mapping structure."""

    keys = sorted(mapping.keys())
    data_json = json.dumps(mapping, indent=2)

    html_content = f"""<!DOCTYPE html>
<html lang=\"en\">
<head>
  <meta charset=\"UTF-8\" />
  <title>Log Viewer</title>
  <style>
    body {{
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }}

    .page {{
      padding: 12px 16px 16px 16px;
    }}

    .toolbar {{
      display: flex;
      align-items: center;
      gap: 8px;
      margin-bottom: 12px;
    }}

    .toolbar label {{
      font-weight: bold;
    }}

    .toolbar input[type='text'] {{
      padding: 4px 6px;
      min-width: 200px;
    }}

    .toolbar select {{
      padding: 4px 6px;
    }}

    .layout {{
      display: grid;
      grid-template-columns: 1.5fr 2fr;
      grid-template-rows: auto auto;
      grid-gap: 8px;
      height: calc(100vh - 70px);
    }}

    .video-panel {{
      border: 2px solid #d33;
      padding: 4px;
    }}

    .video-panel video {{
      width: 100%;
      height: 100%;
      max-height: 100%;
      background: #000;
    }}

    .text-panel {{
      border: 2px solid #999;
      padding: 4px;
      grid-column: 1 / 2;
      grid-row: 2 / 3;
      overflow: auto;
    }}

    .plots-panel {{
      border: 2px solid #3a3;
      padding: 4px;
      grid-column: 2 / 3;
      grid-row: 1 / 3;
      display: grid;
      grid-template-columns: 1fr 1fr;
      grid-template-rows: 1fr 1fr;
      gap: 4px;
    }}

    .plots-panel iframe {{
      width: 100%;
      height: 100%;
      border: 1px solid #ccc;
    }}

    .no-data {{
      color: #666;
      font-style: italic;
    }}
  </style>
</head>
<body>
  <div class=\"page\">
    <div class=\"toolbar\">
      <label for=\"logName\">Log name</label>
      <input id=\"logName\" type=\"text\" readonly value=\"{keys[0] if keys else ''}\" />
      <label for=\"scenarioSelect\">Select scenario</label>
      <select id=\"scenarioSelect\"></select>
    </div>

    <div class=\"layout\">
      <div class=\"video-panel\">
        <video id=\"videoPlayer\" controls></video>
      </div>

      <div class=\"text-panel\" id=\"textPanel\">
        <div class=\"no-data\">Text details on the scenario (from HTML files).</div>
      </div>

      <div class=\"plots-panel\" id=\"htmlContainer\">
        <!-- Iframes with HTML plots will be injected here -->
      </div>
    </div>
  </div>

  <script>
    const mappings = {data_json};

    function populateDropdown() {{
      const select = document.getElementById('scenarioSelect');
      const keys = Object.keys(mappings).sort();
      select.innerHTML = '';

      keys.forEach((key) => {{
        const opt = document.createElement('option');
        opt.value = key;
        opt.textContent = key;
        select.appendChild(opt);
      }});

      if (keys.length > 0) {{
        select.value = keys[0];
        updateScenario(keys[0]);
      }}

      select.addEventListener('change', () => {{
        updateScenario(select.value);
      }});
    }}

    function updateScenario(key) {{
      const entry = mappings[key];
      const videoPlayer = document.getElementById('videoPlayer');
      const htmlContainer = document.getElementById('htmlContainer');
      const logName = document.getElementById('logName');

      logName.value = key;

      if (entry && entry.video) {{
        videoPlayer.src = entry.video;
      }} else {{
        videoPlayer.removeAttribute('src');
      }}

      htmlContainer.innerHTML = '';

      if (entry && Array.isArray(entry.html_files) && entry.html_files.length > 0) {{
        entry.html_files.forEach((path) => {{
          const iframe = document.createElement('iframe');
          iframe.src = path;
          iframe.loading = 'lazy';
          htmlContainer.appendChild(iframe);
        }});
      }} else {{
        const div = document.createElement('div');
        div.className = 'no-data';
        div.textContent = 'No HTML files mapped for this scenario.';
        htmlContainer.appendChild(div);
      }}
    }}

    document.addEventListener('DOMContentLoaded', populateDropdown);
  </script>
</body>
</html>
"""

    with open(output_html, "w", encoding="utf-8") as f:
        f.write(html_content)


def main() -> None:
    if len(sys.argv) < 3:
        print("Usage: python main.py <html_root> <video_root> [output_html]", file=sys.stderr)
        sys.exit(1)

    html_root = sys.argv[1]
    video_root = sys.argv[2]
    output_html = sys.argv[3] if len(sys.argv) >= 4 else "viewer.html"

    if not os.path.isabs(output_html):
        output_html = os.path.abspath(output_html)

    output_dir = os.path.dirname(output_html)

    try:
        html_map = scan_html_root(html_root, output_dir)
        video_map = scan_video_root(video_root, output_dir)
    except FileNotFoundError as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(1)

    html_keys = set(html_map.keys())
    video_keys = set(video_map.keys())

    matched_keys = sorted(html_keys & video_keys)
    only_html = sorted(html_keys - video_keys)
    only_video = sorted(video_keys - html_keys)

    for key in only_html:
        print(f"Mapping not present for video (no video found) for key: {key}")

    for key in only_video:
        print(f"Mapping not present for HTML (no HTML folder found) for key: {key}")

    if not matched_keys:
        print("No matching HTML / video pairs found.")
    else:
        print("Matched keys:")
        for key in matched_keys:
            print(f"  {key} -> {len(html_map[key])} HTML file(s), video: {video_map[key]}")

    mapping: Dict[str, Dict[str, Any]] = {}
    for key in matched_keys:
        mapping[key] = {
            "video": video_map[key],
            "html_files": html_map[key],
        }

    build_viewer_html(output_html, mapping)
    print(f"Viewer HTML generated at: {output_html}")


if __name__ == "__main__":
    main()
