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
        data_json = json.dumps(mapping, indent=2)
        html_template = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>Log Viewer</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    .page {
      padding: 12px 16px 16px 16px;
    }
    .toolbar {
      display: flex;
      align-items: center;
      gap: 8px;
      margin-bottom: 12px;
    }
    .toolbar label {
      font-weight: bold;
    }
    .toolbar select {
      padding: 4px 6px;
    }
    .layout {
      display: grid;
      grid-template-columns: 1fr 1fr;
      grid-template-rows: 1fr 1fr;
      grid-gap: 10px;
      height: calc(100vh - 70px);
    }
    .box {
      border-radius: 4px;
      padding: 8px;
      box-sizing: border-box;
      background: #f5f5f5;
      overflow: hidden;
    }
    .video-box {
      border: 2px solid #ff7f50;
      display: flex;
      flex-direction: column;
    }
    .video-box video {
      width: 100%;
      height: 100%;
      max-height: 100%;
      background: #000;
    }
    .video-info {
      margin-top: 4px;
      font-size: 12px;
      color: #333;
    }
    .image-box {
      border: 2px solid #1e90ff;
      display: flex;
      flex-direction: column;
    }
    .sensor-grid {
      display: grid;
      grid-template-columns: repeat(2, minmax(120px, 1fr));
      grid-auto-rows: minmax(80px, auto);
      gap: 8px;
      overflow: auto;
    }
    .sensor-card {
      background: #ffffff;
      border: 1px solid #ccc;
      border-radius: 4px;
      padding: 4px;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      font-size: 12px;
    }
    .sensor-card img {
      max-width: 100%;
      max-height: 80px;
      object-fit: contain;
      margin-bottom: 4px;
    }
    .text-box {
      border: 2px solid #708090;
      display: flex;
      flex-direction: column;
    }
    #commentBox {
      flex: 1;
      width: 100%;
      resize: none;
      padding: 6px;
      box-sizing: border-box;
      font-family: monospace;
      font-size: 13px;
    }
    .text-actions {
      margin-top: 6px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 8px;
    }
    .text-actions button {
      padding: 4px 10px;
      border-radius: 3px;
      border: none;
      cursor: pointer;
      background: #32cd32;
      color: #fff;
      font-weight: bold;
    }
    .text-path {
      font-size: 11px;
      color: #555;
      flex: 1;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }
    .html-box {
      border: 2px solid #ff69b4;
      display: flex;
      flex-direction: column;
    }
    .html-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 8px;
      margin-bottom: 6px;
      font-size: 13px;
    }
    .html-header button {
      padding: 4px 8px;
      cursor: pointer;
    }
    #htmlPreview {
      flex: 1;
      width: 100%;
      border: 1px solid #ccc;
      background: #fff;
    }
    .no-data {
      color: #666;
      font-style: italic;
    }
  </style>
</head>
<body>
  <div class="page">
    <div class="toolbar">
      <label for="logSelect">Log name</label>
      <select id="logSelect"></select>
      <div id="htmlInfo" class="no-data">No HTML selected.</div>
    </div>

    <div class="layout">
      <!-- Left upper: video player -->
      <div class="box video-box">
        <video id="videoPlayer" controls></video>
        <div id="videoInfo" class="video-info no-data"></div>
      </div>

      <!-- Right upper: images for FL/FR/RL/RR(/FC) -->
      <div class="box image-box">
        <div id="sensorGrid" class="sensor-grid"></div>
      </div>

      <!-- Left lower: text comments with Save -->
      <div class="box text-box">
        <textarea id="commentBox"></textarea>
        <div class="text-actions">
          <div id="commentPath" class="text-path no-data">No video selected.</div>
          <button id="saveCommentBtn" style="display:none;">Save</button>
        </div>
      </div>

      <!-- Right lower: HTML preview + open button -->
      <div class="box html-box">
        <div class="html-header">
          <span id="htmlPreviewLabel" class="no-data">No HTML preview.</span>
          <button id="openHtmlBtn" disabled>Open</button>
        </div>
        <iframe id="htmlPreview"></iframe>
      </div>
    </div>
  </div>

  <script>
    const mappings = {{DATA_JSON}};

    let currentKey = null;
    let currentPreviewHtml = null;
    let isCommentDirty = false;

    function populateDropdown() {
      const select = document.getElementById('logSelect');
      const keys = Object.keys(mappings).sort();
      select.innerHTML = '';
      keys.forEach(function(k) {
        const opt = document.createElement('option');
        opt.value = k;
        opt.textContent = mappings[k].html_folder || k;
        select.appendChild(opt);
      });
      select.addEventListener('change', function() {
        updateScenario(select.value);
      });
      if (keys.length > 0) {
        updateScenario(keys[0]);
      }
    }

    function buildSensorGrid(entry) {
      const grid = document.getElementById('sensorGrid');
      grid.innerHTML = '';
      if (!entry || !Array.isArray(entry.images) || entry.images.length === 0) {
        const msg = document.createElement('div');
        msg.className = 'no-data';
        msg.textContent = 'No images for this scenario.';
        grid.appendChild(msg);
        return;
      }

      const sensors = ['FL', 'FR', 'RL', 'RR', 'FC'];
      let used = 0;
      sensors.forEach(function(sensor) {
        const imgPath = entry.images.find(function(p) { return p.indexOf('/' + sensor + '/') !== -1; });
        if (!imgPath) {
          return;
        }
        used += 1;
        const card = document.createElement('div');
        card.className = 'sensor-card';
        const img = document.createElement('img');
        img.src = imgPath;
        img.alt = imgPath.split('/').pop() || sensor;
        const label = document.createElement('div');
        label.textContent = sensor;
        card.appendChild(img);
        card.appendChild(label);
        grid.appendChild(card);
      });

      if (used === 0) {
        const msg = document.createElement('div');
        msg.className = 'no-data';
        msg.textContent = 'No FL/FR/RL/RR/FC images for this log.';
        grid.appendChild(msg);
      }
    }

    function getCommentPath(entry) {
      if (!entry || !entry.comment_path) return null;
      return entry.comment_path;
    }

    async function loadExistingComment(entry) {
      const commentBox = document.getElementById('commentBox');
      const pathLabel = document.getElementById('commentPath');
      const saveBtn = document.getElementById('saveCommentBtn');
      const path = getCommentPath(entry);
      if (!path) {
        commentBox.value = '';
        pathLabel.textContent = 'No video selected.';
        pathLabel.classList.add('no-data');
        saveBtn.style.display = 'none';
        isCommentDirty = false;
        return;
      }

      pathLabel.textContent = path;
      pathLabel.classList.remove('no-data');
      saveBtn.style.display = 'none';
      isCommentDirty = false;

      try {
        const resp = await fetch(path);
        if (!resp.ok) throw new Error('not found');
        const txt = await resp.text();
        commentBox.value = txt;
      } catch (e) {
        commentBox.value = '';
      }
    }

    async function saveComment(entry) {
      const commentBox = document.getElementById('commentBox');
      const saveBtn = document.getElementById('saveCommentBtn');
      const path = getCommentPath(entry);
      if (!path) return;

      const body = commentBox.value || '';

      // Pure-browser save: download a .txt file which you can place under db/video/log_txt/
      const blob = new Blob([body], { type: 'text/plain' });
      const a = document.createElement('a');
      a.href = URL.createObjectURL(blob);
      a.download = path.split('/').pop() || 'comment.txt';
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(a.href);
      isCommentDirty = false;
      saveBtn.style.display = 'none';
    }

    function updateHtmlPreview(entry, key) {
      const iframe = document.getElementById('htmlPreview');
      const label = document.getElementById('htmlPreviewLabel');
      const openBtn = document.getElementById('openHtmlBtn');
      const htmlInfo = document.getElementById('htmlInfo');

      if (entry && Array.isArray(entry.html_files) && entry.html_files.length > 0) {
        const mainHtml = entry.html_files[0];
        currentPreviewHtml = mainHtml;
        iframe.src = mainHtml;
        label.textContent = mainHtml.split('/').pop() || 'HTML preview';
        label.classList.remove('no-data');
        openBtn.disabled = false;
        if (htmlInfo) {
          htmlInfo.textContent = entry.html_folder || key;
          htmlInfo.classList.remove('no-data');
        }
      } else {
        currentPreviewHtml = null;
        iframe.removeAttribute('src');
        label.textContent = 'No HTML preview.';
        label.classList.add('no-data');
        openBtn.disabled = true;
        if (htmlInfo) {
          htmlInfo.textContent = 'No HTML files mapped for this scenario.';
          htmlInfo.classList.add('no-data');
        }
      }
    }

    function wireGlobalHandlers() {
      const openBtn = document.getElementById('openHtmlBtn');
      openBtn.addEventListener('click', function() {
        if (currentPreviewHtml) {
          window.open(currentPreviewHtml, '_blank');
        }
      });

      const commentBox = document.getElementById('commentBox');
      const saveBtn = document.getElementById('saveCommentBtn');
      commentBox.addEventListener('input', function() {
        isCommentDirty = true;
        saveBtn.style.display = 'inline-block';
      });
      saveBtn.addEventListener('click', function() {
        if (!currentKey) return;
        const entry = mappings[currentKey];
        saveComment(entry);
      });
    }

    function updateScenario(key) {
      currentKey = key;
      const entry = mappings[key];
      const videoPlayer = document.getElementById('videoPlayer');
      const videoInfo = document.getElementById('videoInfo');

      if (entry && entry.video) {
        videoPlayer.src = entry.video;
        videoPlayer.autoplay = true;
        videoPlayer.load();
        videoPlayer.play().catch(function() {});
        videoInfo.textContent = entry.video_name || entry.video;
        videoInfo.classList.remove('no-data');
      } else {
        videoPlayer.removeAttribute('src');
        videoInfo.textContent = 'No video for this scenario.';
        videoInfo.classList.add('no-data');
      }

      buildSensorGrid(entry);
      loadExistingComment(entry);
      updateHtmlPreview(entry, key);
    }

    document.addEventListener('DOMContentLoaded', function() {
      wireGlobalHandlers();
      populateDropdown();
    });
  </script>
</body>
</html>
"""
        html = html_template.replace("{{DATA_JSON}}", data_json)
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
                base_name, _ext = os.path.splitext(video_name)
                comment_path = f"db/video/log_txt/{base_name}.txt"
                mapping[k] = {
                    "video": video_url,
                    "video_name": video_name,
                    "html_files": html_files,
                    "html_folder": html_folder,
                    "images": images,
                    "comment_path": comment_path,
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