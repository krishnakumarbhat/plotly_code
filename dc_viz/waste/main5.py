import os
import sys
import json
from typing import Any, Dict, List, Optional


class LogViewerApp:
    def __init__(self, html_root: str, video_root: str, output_html: str = "viewer.html") -> None:
        self.html_root = os.path.abspath(html_root)
        self.video_root = os.path.abspath(video_root)
        self.output_html = os.path.abspath(output_html)
        self.output_dir = os.path.dirname(self.output_html)

    @staticmethod
    def _key(name: str) -> Optional[str]:
        base = os.path.splitext(os.path.basename(name))[0]
        # Remove _web suffix if present (for video files)
        if base.endswith("_web"):
            base = base[:-4]
        parts = base.split("_")
        if len(parts) < 2:
            return None
        return "_".join(parts[-2:])

    def _scan_html(self) -> Dict[str, List[str]]:
        if not os.path.isdir(self.html_root):
            raise FileNotFoundError(self.html_root)
        m: Dict[str, List[str]] = {}
        for entry in os.scandir(self.html_root):
            if not entry.is_dir():
                continue
            k = self._key(entry.name)
            if not k:
                continue
            files: List[str] = []
            for root, _d, fnames in os.walk(entry.path):
                for f in fnames:
                    if os.path.splitext(f)[1].lower() not in {".html", ".htm"}:
                        continue
                    full = os.path.join(root, f)
                    rel = os.path.relpath(full, self.output_dir).replace("\\", "/")
                    files.append(rel)
            if files:
                files.sort()
                m[k] = files
        return m

    def _scan_images(self) -> Dict[str, List[str]]:
        if not os.path.isdir(self.html_root):
            return {}
        img_map: Dict[str, List[str]] = {}
        for entry in os.scandir(self.html_root):
            if not entry.is_dir():
                continue
            k = self._key(entry.name)
            if not k:
                continue
            imgs: List[str] = []
            for root, _d, fnames in os.walk(entry.path):
                for f in fnames:
                    if os.path.splitext(f)[1].lower() not in {".png"}:
                        continue
                    full = os.path.join(root, f)
                    rel = os.path.relpath(full, self.output_dir).replace("\\", "/")
                    imgs.append(rel)
            if imgs:
                imgs.sort()
                img_map[k] = imgs
        return img_map

    def _scan_video(self) -> Dict[str, str]:
        if not os.path.isdir(self.video_root):
            return {}
        exts = {".mp4", ".avi", ".mov", ".mkv"}
        m: Dict[str, str] = {}
        for entry in os.scandir(self.video_root):
            if not entry.is_file():
                continue
            if os.path.splitext(entry.name)[1].lower() not in exts:
                continue
            k = self._key(entry.name)
            if not k:
                continue
            abs_path = os.path.abspath(entry.path).replace("\\", "/")
            m[k] = f"file:///{abs_path}"
        return m

    def _get_html_folder_name(self, key: str) -> str:
        """Get the HTML folder name for a given key."""
        for entry in os.scandir(self.html_root):
            if entry.is_dir() and self._key(entry.name) == key:
                return entry.name
        return key

    @staticmethod
    def _build_html(out_path: str, mapping: Dict[str, Dict[str, Any]]) -> None:
        keys = sorted(mapping.keys())
        data_json = json.dumps(mapping, indent=2)
        html = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
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
      grid-template-rows: auto;
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
    .links-panel {{
      border: 2px solid #3a3;
      padding: 8px;
      overflow: auto;
    }}
    .links-panel h3 {{
      margin-top: 0;
    }}
    .html-buttons {{
      display: flex;
      flex-wrap: wrap;
      gap: 6px;
    }}
    .html-buttons button {{
      padding: 4px 8px;
      cursor: pointer;
    }}
    .no-data {{
      color: #666;
      font-style: italic;
    }}
    .image-strip {{
      display: flex;
      flex-wrap: wrap;
      gap: 4px;
      margin-bottom: 8px;
    }}
    .image-strip img {{
      max-width: 120px;
      max-height: 90px;
      object-fit: cover;
      border: 1px solid #ccc;
    }}
    .html-info {{
      font-size: 12px;
      margin-top: 4px;
      color: #333;
    }}
  </style>
</head>
<body>
  <div class="page">
    <div class="toolbar">
      <label for="logSelect">Log name</label>
      <select id="logSelect"></select>
      <div id="htmlInfo" class="html-info no-data">No HTML selected.</div>
    </div>

    <div class="layout">
      <div class="video-panel">
        <video id="videoPlayer" controls></video>
        <div id="videoInfo" class="no-data"></div>
      </div>

      <div class="links-panel">
        <h3>HTML views</h3>
        <div id="imageStrip" class="image-strip"></div>
        <div id="htmlButtons" class="html-buttons"></div>
        <div id="noHtmlMsg" class="no-data" style="display:none;">
          No HTML files mapped for this scenario.
        </div>
      </div>
    </div>
  </div>

  <script>
    const mappings = {data_json};

    function populateDropdown() {{
      const select = document.getElementById('logSelect');
      const keys = Object.keys(mappings).sort();
      select.innerHTML = '';
      keys.forEach((k) => {{
        const opt = document.createElement('option');
        opt.value = k;
        opt.textContent = mappings[k].html_folder || k;
        select.appendChild(opt);
      }});
      select.addEventListener('change', () => {{
        updateScenario(select.value);
      }});
      if (keys.length > 0) {{
        updateScenario(keys[0]);
      }}
    }}

    function updateScenario(key) {{
      const entry = mappings[key];
      const videoPlayer = document.getElementById('videoPlayer');
      const btnContainer = document.getElementById('htmlButtons');
      const noHtmlMsg = document.getElementById('noHtmlMsg');
      const videoInfo = document.getElementById('videoInfo');
      const htmlInfo = document.getElementById('htmlInfo');
      const imageStrip = document.getElementById('imageStrip');

      if (entry && entry.video) {{
        videoPlayer.src = entry.video;
        videoPlayer.muted = true;
        videoPlayer.autoplay = true;
        videoPlayer.load();
        videoPlayer.play().catch(() => {{}});
        if (videoInfo) {{
          videoInfo.textContent = entry.video_name || entry.video;
        }}
      }} else {{
        videoPlayer.removeAttribute('src');
        if (videoInfo) {{
          videoInfo.textContent = 'No video for this scenario.';
        }}
      }}

      btnContainer.innerHTML = '';
      imageStrip.innerHTML = '';

        if (entry && Array.isArray(entry.html_files) && entry.html_files.length > 0) {{
        noHtmlMsg.style.display = 'none';
        if (htmlInfo) {{
          htmlInfo.textContent = entry.html_folder || key;
          htmlInfo.classList.remove('no-data');
        }}        if (Array.isArray(entry.images) && entry.images.length > 0) {{
          entry.images.forEach((path) => {{
            const img = document.createElement('img');
            img.src = path;
            img.alt = path.split('/').pop() || 'image';
            imageStrip.appendChild(img);
          }});
        }}

        btnContainer.style.display = 'flex';
        entry.html_files.forEach((path, i) => {{
          const btn = document.createElement('button');
          const name = path.split('/').pop() || ('HTML ' + (i + 1));
          btn.textContent = name;
          btn.addEventListener('click', () => {{
            window.open(path, '_blank');
          }});
          btnContainer.appendChild(btn);
        }});
      }} else {{
        noHtmlMsg.style.display = 'block';
        if (htmlInfo) {{
          htmlInfo.textContent = 'No HTML files mapped for this scenario.';
          htmlInfo.classList.add('no-data');
        }}
        btnContainer.style.display = 'none';
      }}
    }}

    document.addEventListener('DOMContentLoaded', populateDropdown);
  </script>
</body>
</html>
"""
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(html)

    def run(self) -> None:
        html_map = self._scan_html()
        img_map = self._scan_images()
        video_map = self._scan_video()

        html_keys = set(html_map)
        video_keys = set(video_map)

        matched = sorted(html_keys & video_keys)
        only_html = sorted(html_keys - video_keys)
        only_video = sorted(video_keys - html_keys)

        for k in only_html:
            print(f"Mapping not present for video (no video found) for key or ffmeg conve not happed for this to codec support : {k}")
        for k in only_video:
            print(f"Mapping not present for HTML (no HTML folder found) for key: {k}")

        mapping: Dict[str, Dict[str, Any]] = {}
        if not matched:
            print("No matching HTML / video pairs found.")
        else:
            print("Matched keys:")
            for k in matched:
                html_files = html_map[k]
                video_url = video_map[k]
                images = img_map.get(k, [])

                # Derive friendly names
                html_folder = self._get_html_folder_name(k)
                video_path = video_url.replace("file:///", "") if video_url.startswith("file:///") else video_url
                video_name = os.path.basename(video_path.replace("\\", "/"))

                print(f"  {k} -> {len(html_files)} HTML file(s), video: {video_url}")
                mapping[k] = {
                    "video": video_url,
                    "video_name": video_name,
                    "html_files": html_files,
                    "html_folder": html_folder,
                    "images": images,
                }

        self._build_html(self.output_html, mapping)
        print(f"Viewer HTML generated at: {self.output_html}")


def main() -> None:
    if len(sys.argv) < 3:
        print("Usage: python main.py <html_root> <video_root> [output_html]", file=sys.stderr)
        sys.exit(1)
    html_root = sys.argv[1]
    video_root = sys.argv[2]
    output_html = sys.argv[3] if len(sys.argv) >= 4 else "viewer.html"
    app = LogViewerApp(html_root, video_root, output_html)
    app.run()


if __name__ == "__main__":
    main()