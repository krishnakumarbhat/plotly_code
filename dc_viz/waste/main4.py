import os
import ffmpeg
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

    def _scan_video(self) -> Dict[str, str]:
        brow_root = os.path.join(self.video_root, "brow_video")
        if not os.path.isdir(brow_root):
            return {}
        exts = {".mp4", ".avi", ".mov", ".mkv"}
        m: Dict[str, str] = {}
        for entry in os.scandir(brow_root):
            if not entry.is_file():
                continue
            if os.path.splitext(entry.name)[1].lower() not in exts:
                continue
            # Skip _web converted files (they are handled by _convert_videos)
            if "_web" in entry.name:
                continue
            k = self._key(entry.name)
            if not k:
                continue
            abs_path = os.path.abspath(entry.path).replace("\\", "/")
            m[k] = f"file:///{abs_path}"
        return m


    def _convert_videos(self, video_map: Dict[str, str]) -> Dict[str, str]:
        out_root = os.path.join(self.video_root, "brow_video")
        os.makedirs(out_root, exist_ok=True)
        converted: Dict[str, str] = {}

        for key, src in video_map.items():
            src_path = src
            if not os.path.isfile(src_path):
                # handle "file:///" style paths
                src_path = os.path.abspath(src_path.replace("file:///", ""))

            abs_src = os.path.abspath(src_path).replace("\\", "/")
            fallback_url = f"file:///{abs_src}"

            base = os.path.splitext(os.path.basename(src_path))[0]
            ext = os.path.splitext(src_path)[1].lower()

            if ext in {".mp4", ".avi", ".mov", ".mkv"}:
                out_ext = ".mp4"
                out_name = f"{base}_web{out_ext}"
                out_path = os.path.join(out_root, out_name)

                if not os.path.isfile(out_path):
                    try:
                        # Use ffmpeg-python instead of subprocess cmd
                        (
                            ffmpeg
                            .input(src_path)
                            .output(
                                out_path,
                                vcodec="libx264",
                                preset="veryfast",
                                crf=23,
                                acodec="aac",
                                audio_bitrate="128k",
                            )
                            .overwrite_output()
                            .run(quiet=True)
                        )
                    except FileNotFoundError:
                        print(f"Warning: FFmpeg not installed. Skipping conversion for: {key}")
                        converted[key] = fallback_url
                        continue
                    except Exception as e:
                        print(f"Warning: FFmpeg conversion failed for {key}: {e}")
                        converted[key] = fallback_url
                        continue

                abs_conv = os.path.abspath(out_path).replace("\\", "/")
                converted[key] = f"file:///{abs_conv}"
                continue

            converted[key] = fallback_url

        return converted


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
  </style>
</head>
<body>
  <div class="page">
    <div class="toolbar">
      <label for="logName">Log name</label>
      <input id="logName" type="text" readonly value="{keys[0] if keys else ''}" />
      <label for="scenarioSelect">Select scenario</label>
      <select id="scenarioSelect"></select>
    </div>

    <div class="layout">
      <div class="video-panel">
        <video id="videoPlayer" controls></video>
        <div id="videoInfo" class="no-data"></div>
      </div>

      <div class="links-panel">
        <h3>HTML views</h3>
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
      const select = document.getElementById('scenarioSelect');
      const keys = Object.keys(mappings).sort();
      select.innerHTML = '';
      keys.forEach((k) => {{
        const opt = document.createElement('option');
        opt.value = k;
        opt.textContent = k;
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
      const logName = document.getElementById('logName');
      const btnContainer = document.getElementById('htmlButtons');
      const noHtmlMsg = document.getElementById('noHtmlMsg');
      const videoInfo = document.getElementById('videoInfo');

      logName.value = key || '';

      if (entry && entry.video) {{
        videoPlayer.src = entry.video;
        videoPlayer.muted = true;
        videoPlayer.autoplay = true;
        videoPlayer.load();
        videoPlayer.play().catch(() => {{}});
        if (videoInfo) {{
          videoInfo.textContent = entry.video;
        }}
      }} else {{
        videoPlayer.removeAttribute('src');
        if (videoInfo) {{
          videoInfo.textContent = 'No video for this scenario.';
        }}
      }}

      btnContainer.innerHTML = '';
      if (entry && Array.isArray(entry.html_files) && entry.html_files.length > 0) {{
        noHtmlMsg.style.display = 'none';
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
      }}
    }}

    document.addEventListener('DOMContentLoaded', populateDropdown);
  </script>
</body>
</html>
"""
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(html)

    def _scan_video_source(self) -> Dict[str, str]:
        """Scan db/video for source videos (not brow_video)."""
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
            m[k] = os.path.abspath(entry.path)
        return m

    def run(self) -> None:
        html_map = self._scan_html()
        
        # First scan source videos in db/video
        source_video_map = self._scan_video_source()
        
        # Convert source videos to brow_video folder
        if source_video_map:
            print(f"Found {len(source_video_map)} source video(s) to convert...")
            converted_video_map = self._convert_videos(source_video_map)
        else:
            converted_video_map = {}
        
        # Also check existing videos in brow_video
        existing_video_map = self._scan_video()
        
        # Merge: converted videos take priority, then existing
        video_map = {**existing_video_map, **converted_video_map}

        html_keys = set(html_map)
        video_keys = set(video_map)

        matched = sorted(html_keys & video_keys)
        only_html = sorted(html_keys - video_keys)
        only_video = sorted(video_keys - html_keys)

        for k in only_html:
            print(f"Mapping not present for video (no video found) for key or ffmeg conve not happed for this to codec support : {k}")
        for k in only_video:
            print(f"Mapping not present for HTML (no HTML folder found) for key: {k}")

        if not matched:
            print("No matching HTML / video pairs found.")
            mapping: Dict[str, Dict[str, Any]] = {}
        else:
            print("Matched keys:")
            mapping = {}
            for k in matched:
                print(f"  {k} -> {len(html_map[k])} HTML file(s), video: {video_map[k]}")
                mapping[k] = {"video": video_map[k], "html_files": html_map[k]}

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