import os
import sys
import json
from typing import Any, Dict, List, Optional

# ./llama-cli -m Qwen3-VL-2B-Instruct-Q5_K_M.gguf -p "Describe this image" --mmproj mmproj-model-f16.gguf --image test.jpg


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
        text_map = self._scan_text()

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
        # Remove common suffixes like _web if present
        suffixes_to_remove = ["_web"]
        for suffix in suffixes_to_remove:
            if base.endswith(suffix):
                base = base[:-len(suffix)]
                break
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

    def _scan_images(self) -> Dict[str, Dict[str, Dict[str, str]]]:
        """Scan images and organize by key -> sensor_name -> sensor_position.
        
        sensor_name: extracted from image filename (e.g., range, velocity, rangerate), 
                     defaults to "other" if not found
        sensor_position: extracted from folder name or image filename (e.g., FL, FR, RL, RR, FC)
        """
        if not os.path.isdir(self.html_root):
            return {}
        
        # Known sensor names to look for in image filenames
        sensor_names = {"range", "velocity", "rangerate", "doppler", "azimuth", "elevation", "snr", "rcs"}
        # Known sensor positions
        sensor_positions = {"FL", "FR", "RL", "RR", "FC"}
        
        img_map: Dict[str, Dict[str, Dict[str, str]]] = {}
        
        for entry in os.scandir(self.html_root):
            if not entry.is_dir():
                continue
            k = self._key(entry.name)
            if not k:
                continue
            
            imgs: Dict[str, Dict[str, str]] = {}
            
            for root, _d, fnames in os.walk(entry.path):
                for f in fnames:
                    if os.path.splitext(f)[1].lower() not in {".png"}:
                        continue
                    
                    full = os.path.join(root, f)
                    rel = os.path.relpath(full, self.output_dir).replace("\\", "/")
                    
                    # Extract sensor position from folder name or image name
                    folder_name = os.path.basename(root).upper()
                    file_base = os.path.splitext(f)[0].lower()
                    
                    # Try to find sensor position in folder name first, then image name
                    sensor_pos = None
                    for pos in sensor_positions:
                        if folder_name == pos:
                            sensor_pos = pos
                            break
                        if pos.lower() in file_base or f"_{pos.lower()}" in file_base:
                            sensor_pos = pos
                            break
                    
                    # If no sensor position found, skip this image
                    if not sensor_pos:
                        continue
                    
                    # Try to find sensor name in image filename
                    sensor_name = "other"
                    for sn in sensor_names:
                        if sn in file_base:
                            sensor_name = sn
                            break
                    
                    # Initialize nested dict if needed
                    if sensor_name not in imgs:
                        imgs[sensor_name] = {}
                    
                    imgs[sensor_name][sensor_pos] = rel
            
            if imgs:
                img_map[k] = imgs
        
        return img_map

    def _scan_text(self) -> Dict[str, Dict[str, str]]:
        """Scan text files in video/log_txt directory and return mapping of key to {path, content}."""
        text_dir = os.path.join(self.video_root, "log_txt")
        if not os.path.isdir(text_dir):
            return {}
        
        m: Dict[str, Dict[str, str]] = {}
        for entry in os.scandir(text_dir):
            if not entry.is_file():
                continue
            if os.path.splitext(entry.name)[1].lower() != ".txt":
                continue
            k = self._key(entry.name)
            if not k:
                continue
            rel = os.path.relpath(entry.path, self.output_dir).replace("\\", "/")
            # Read the content of the text file
            try:
                with open(entry.path, "r", encoding="utf-8") as f:
                    content = f.read()
            except Exception:
                content = ""
            m[k] = {"path": rel, "content": content}
        return m

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
    .image-header {
      display: flex;
      align-items: center;
      gap: 8px;
      margin-bottom: 6px;
      font-size: 13px;
    }
    .image-header label {
      font-weight: bold;
    }
    .image-header select {
      padding: 4px 6px;
    }
    .sensor-grid {
      display: grid;
      grid-template-columns: repeat(2, minmax(120px, 1fr));
      grid-auto-rows: minmax(80px, auto);
      gap: 8px;
      overflow: auto;
      flex: 1;
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
      cursor: pointer;
      transition: transform 0.15s, box-shadow 0.15s;
    }
    .sensor-card:hover {
      transform: scale(1.02);
      box-shadow: 0 2px 8px rgba(0,0,0,0.15);
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
      color: #fff;
      font-weight: bold;
    }
    #editCommentBtn {
      background: #1e90ff;
    }
    #saveCommentBtn {
      background: #32cd32;
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
    /* Modal / Lightbox styles */
    .modal-overlay {
      display: none;
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: rgba(0, 0, 0, 0.85);
      z-index: 1000;
      justify-content: center;
      align-items: center;
      flex-direction: column;
    }
    .modal-overlay.active {
      display: flex;
    }
    .modal-content {
      position: relative;
      max-width: 90%;
      max-height: 85%;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .modal-content img {
      max-width: 100%;
      max-height: 80vh;
      object-fit: contain;
      border-radius: 4px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.5);
    }
    .modal-label {
      color: #fff;
      margin-top: 12px;
      font-size: 16px;
      text-align: center;
    }
    .modal-close {
      position: absolute;
      top: -40px;
      right: -10px;
      background: transparent;
      border: none;
      color: #fff;
      font-size: 32px;
      cursor: pointer;
      padding: 8px;
    }
    .modal-close:hover {
      color: #ff6b6b;
    }
    .modal-nav {
      position: absolute;
      top: 50%;
      transform: translateY(-50%);
      background: rgba(255,255,255,0.2);
      border: none;
      color: #fff;
      font-size: 28px;
      cursor: pointer;
      padding: 16px 12px;
      border-radius: 4px;
    }
    .modal-nav:hover {
      background: rgba(255,255,255,0.4);
    }
    .modal-prev {
      left: -60px;
    }
    .modal-next {
      right: -60px;
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
        <div class="image-header">
          <label for="sensorSelect">Sensor:</label>
          <select id="sensorSelect">
            <option value="all">All</option>
          </select>
        </div>
        <div id="sensorGrid" class="sensor-grid"></div>
      </div>

      <!-- Left lower: text comments with Edit/Save -->
      <div class="box text-box">
        <textarea id="commentBox" readonly></textarea>
        <div class="text-actions">
          <div id="commentPath" class="text-path no-data">No video selected.</div>
          <button id="editCommentBtn">Edit</button>
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

  <!-- Modal for enlarged images -->
  <div id="imageModal" class="modal-overlay">
    <div class="modal-content">
      <button class="modal-close" id="modalClose">&times;</button>
      <button class="modal-nav modal-prev" id="modalPrev">&#10094;</button>
      <img id="modalImage" src="" alt="Enlarged view" />
      <button class="modal-nav modal-next" id="modalNext">&#10095;</button>
      <div class="modal-label" id="modalLabel"></div>
    </div>
  </div>

  <script>
    const mappings = {{DATA_JSON}};

    let currentKey = null;
    let currentPreviewHtml = null;
    let isCommentDirty = false;
    let currentSensorFilter = 'all';
    let currentImages = []; // Array of {src, label} for modal navigation
    let currentModalIndex = 0;

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

    function populateSensorDropdown(entry) {
      const select = document.getElementById('sensorSelect');
      select.innerHTML = '<option value="all">All</option>';
      
      if (!entry || !entry.images || typeof entry.images !== 'object') {
        return;
      }
      
      const sensorNames = Object.keys(entry.images).sort();
      sensorNames.forEach(function(sn) {
        const opt = document.createElement('option');
        opt.value = sn;
        opt.textContent = sn.charAt(0).toUpperCase() + sn.slice(1);
        select.appendChild(opt);
      });
      
      // Reset filter to 'all' when scenario changes
      select.value = 'all';
      currentSensorFilter = 'all';
    }

    function buildSensorGrid(entry, filterSensor) {
      const grid = document.getElementById('sensorGrid');
      grid.innerHTML = '';
      currentImages = [];
      
      // images is now: { sensor_name: { sensor_position: path } }
      if (!entry || !entry.images || typeof entry.images !== 'object' || Object.keys(entry.images).length === 0) {
        const msg = document.createElement('div');
        msg.className = 'no-data';
        msg.textContent = 'No images for this scenario.';
        grid.appendChild(msg);
        return;
      }

      const positions = ['FL', 'FR', 'RL', 'RR', 'FC'];
      let used = 0;
      
      // Determine which sensor names to show
      let sensorNames = Object.keys(entry.images).sort();
      if (filterSensor && filterSensor !== 'all') {
        sensorNames = sensorNames.filter(function(sn) { return sn === filterSensor; });
      }
      
      sensorNames.forEach(function(sensorName) {
        const positionsData = entry.images[sensorName];
        if (!positionsData || typeof positionsData !== 'object') return;
        
        positions.forEach(function(pos) {
          const imgPath = positionsData[pos];
          if (!imgPath) return;
          
          const imgIndex = currentImages.length;
          const label = sensorName + ' - ' + pos;
          currentImages.push({ src: imgPath, label: label });
          
          used += 1;
          const card = document.createElement('div');
          card.className = 'sensor-card';
          card.setAttribute('data-index', imgIndex);
          card.addEventListener('click', function() {
            openModal(imgIndex);
          });
          
          const img = document.createElement('img');
          img.src = imgPath;
          img.alt = imgPath.split('/').pop() || label;
          const labelDiv = document.createElement('div');
          labelDiv.textContent = label;
          card.appendChild(img);
          card.appendChild(labelDiv);
          grid.appendChild(card);
        });
      });

      if (used === 0) {
        const msg = document.createElement('div');
        msg.className = 'no-data';
        msg.textContent = filterSensor && filterSensor !== 'all' 
          ? 'No images for sensor: ' + filterSensor 
          : 'No FL/FR/RL/RR/FC images for this log.';
        grid.appendChild(msg);
      }
    }

    function openModal(index) {
      if (currentImages.length === 0) return;
      currentModalIndex = index;
      updateModalContent();
      document.getElementById('imageModal').classList.add('active');
      document.body.style.overflow = 'hidden';
    }

    function closeModal() {
      document.getElementById('imageModal').classList.remove('active');
      document.body.style.overflow = '';
    }

    function updateModalContent() {
      const img = document.getElementById('modalImage');
      const label = document.getElementById('modalLabel');
      const prevBtn = document.getElementById('modalPrev');
      const nextBtn = document.getElementById('modalNext');
      
      if (currentImages.length === 0) return;
      
      const current = currentImages[currentModalIndex];
      img.src = current.src;
      label.textContent = current.label + ' (' + (currentModalIndex + 1) + '/' + currentImages.length + ')';
      
      // Show/hide nav buttons based on position
      prevBtn.style.visibility = currentModalIndex > 0 ? 'visible' : 'hidden';
      nextBtn.style.visibility = currentModalIndex < currentImages.length - 1 ? 'visible' : 'hidden';
    }

    function navigateModal(direction) {
      const newIndex = currentModalIndex + direction;
      if (newIndex >= 0 && newIndex < currentImages.length) {
        currentModalIndex = newIndex;
        updateModalContent();
      }
    }

    function getCommentPath(entry) {
      if (!entry || !entry.comment_path) return null;
      return entry.comment_path;
    }

    function loadExistingComment(entry) {
      const commentBox = document.getElementById('commentBox');
      const pathLabel = document.getElementById('commentPath');
      const saveBtn = document.getElementById('saveCommentBtn');
      const editBtn = document.getElementById('editCommentBtn');
      const path = getCommentPath(entry);
      
      // Reset to read-only mode
      commentBox.readOnly = true;
      saveBtn.style.display = 'none';
      editBtn.style.display = 'inline-block';
      isCommentDirty = false;
      
      if (!path) {
        commentBox.value = '';
        pathLabel.textContent = 'No video selected.';
        pathLabel.classList.add('no-data');
        return;
      }

      pathLabel.textContent = path;
      pathLabel.classList.remove('no-data');

      // Use embedded content instead of fetching (fetch doesn't work with file:// protocol)
      commentBox.value = entry.comment_content || '';
    }

    async function saveComment(entry) {
      const commentBox = document.getElementById('commentBox');
      const saveBtn = document.getElementById('saveCommentBtn');
      const editBtn = document.getElementById('editCommentBtn');
      const path = getCommentPath(entry);
      if (!path) return;

      const body = commentBox.value || '';
      const defaultFileName = path.split('/').pop() || 'comment.txt';
      
      // Get absolute path from entry
      const absPath = entry.comment_abs_path || path;
      
      // Show full absolute path info and prompt for confirmation
      const message = 'Save location to overwrite original:\\n' + absPath + '\\n\\n' +
                      'Click OK to download. After download, move the file to the path above to overwrite.\\n\\n' +
                      'File name:';
      const fileName = prompt(message, defaultFileName);
      if (!fileName) {
        // User cancelled
        return;
      }

      // Browser save: triggers download dialog where user can choose location
      const blob = new Blob([body], { type: 'text/plain' });
      const a = document.createElement('a');
      a.href = URL.createObjectURL(blob);
      a.download = fileName;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(a.href);
      
      // Show reminder about where to place the file
      alert('File downloaded!\\n\\nTo overwrite the original, copy/move the downloaded file to:\\n\\n' + absPath);
      
      // Reset to read-only mode after save
      isCommentDirty = false;
      commentBox.readOnly = true;
      saveBtn.style.display = 'none';
      editBtn.style.display = 'inline-block';
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
      const editBtn = document.getElementById('editCommentBtn');
      
      // Edit button enables editing
      editBtn.addEventListener('click', function() {
        commentBox.readOnly = false;
        commentBox.focus();
        editBtn.style.display = 'none';
        saveBtn.style.display = 'inline-block';
      });
      
      // Track changes
      commentBox.addEventListener('input', function() {
        isCommentDirty = true;
      });
      
      // Save button - prompts user to download file
      saveBtn.addEventListener('click', function() {
        if (!currentKey) return;
        const entry = mappings[currentKey];
        saveComment(entry);
      });

      // Sensor dropdown filter
      const sensorSelect = document.getElementById('sensorSelect');
      sensorSelect.addEventListener('change', function() {
        currentSensorFilter = sensorSelect.value;
        if (currentKey) {
          const entry = mappings[currentKey];
          buildSensorGrid(entry, currentSensorFilter);
        }
      });

      // Modal handlers
      document.getElementById('modalClose').addEventListener('click', closeModal);
      document.getElementById('modalPrev').addEventListener('click', function() {
        navigateModal(-1);
      });
      document.getElementById('modalNext').addEventListener('click', function() {
        navigateModal(1);
      });
      
      // Close modal on overlay click
      document.getElementById('imageModal').addEventListener('click', function(e) {
        if (e.target === this) {
          closeModal();
        }
      });
      
      // Keyboard navigation for modal
      document.addEventListener('keydown', function(e) {
        const modal = document.getElementById('imageModal');
        if (!modal.classList.contains('active')) return;
        
        if (e.key === 'Escape') {
          closeModal();
        } else if (e.key === 'ArrowLeft') {
          navigateModal(-1);
        } else if (e.key === 'ArrowRight') {
          navigateModal(1);
        }
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

      populateSensorDropdown(entry);
      buildSensorGrid(entry, 'all');
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
        text_map = self._scan_text()

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
                images = img_map.get(k, {})

                # Derive friendly names
                html_folder = self._get_html_folder_name(k)
                video_path = video_url.replace("file:///", "") if video_url.startswith("file:///") else video_url
                video_name = os.path.basename(video_path.replace("\\", "/"))

                print(f"  {k} -> {len(html_files)} HTML file(s), video: {video_url}")
                # Use text_map if available, otherwise fall back to derived path
                text_info = text_map.get(k)
                if text_info:
                    comment_path = text_info["path"]
                    comment_content = text_info["content"]
                    # Get absolute path for display
                    comment_abs_path = os.path.abspath(os.path.join(self.output_dir, comment_path)).replace("\\", "/")
                else:
                    base_name, _ext = os.path.splitext(video_name)
                    comment_path = f"db/video/log_txt/{base_name}.txt"
                    comment_content = ""
                    comment_abs_path = os.path.abspath(os.path.join(self.output_dir, comment_path)).replace("\\", "/")
                mapping[k] = {
                    "video": video_url,
                    "video_name": video_name,
                    "html_files": html_files,
                    "html_folder": html_folder,
                    "images": images,
                    "comment_path": comment_path,
                    "comment_abs_path": comment_abs_path,
                    "comment_content": comment_content,
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