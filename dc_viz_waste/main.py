import os
import sys
import threading
import webbrowser
from typing import Any, Dict, List, Optional
from flask import Flask, request, jsonify, send_from_directory, abort
from flask_cors import CORS
from html_build import build_html
from cluster_connect import cluster, SERVERS, download_progress, reset_progress, update_progress

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(BASE_DIR, "out")
DB_DIR = os.path.join(BASE_DIR, "db")

app = Flask(__name__, static_folder=OUT_DIR)
CORS(app)

# Global pointer to latest generated viewer HTML (absolute path)
LATEST_VIEWER_PATH: str | None = None

# Global viewer app instance
viewer_app = None


def set_latest_viewer(path: str) -> None:
    """Call this when a new viewer.html is generated elsewhere."""
    global LATEST_VIEWER_PATH
    if path:
        LATEST_VIEWER_PATH = os.path.abspath(path)
        print(f"Latest viewer set to: {LATEST_VIEWER_PATH}")


def get_latest_viewer() -> str | None:
    return LATEST_VIEWER_PATH


class LogViewerApp:
    def __init__(self, html_root: str, video_root: str, output_html: str = "out/viewer.html") -> None:
        self.html_root = os.path.abspath(html_root)
        self.video_root = os.path.abspath(video_root)
        self.output_html = os.path.abspath(output_html)
        self.output_dir = os.path.dirname(self.output_html)

    @staticmethod
    def _key(name: str) -> Optional[str]:
        base = os.path.splitext(os.path.basename(name))[0]
        for suffix in ["_web"]:
            if base.endswith(suffix):
                base = base[:-len(suffix)]
                break
        parts = base.split("_")
        return "_".join(parts[-2:]) if len(parts) >= 2 else None

    def _scan_html(self, serve_mode: bool = False) -> Dict[str, List[str]]:
        if not os.path.isdir(self.html_root):
            raise FileNotFoundError(self.html_root)
        m: Dict[str, List[str]] = {}
        for entry in os.scandir(self.html_root):
            if not entry.is_dir():
                continue
            k = self._key(entry.name)
            if not k:
                continue
            files = []
            for root, _, fnames in os.walk(entry.path):
                for f in fnames:
                    if os.path.splitext(f)[1].lower() in {".html", ".htm"}:
                        full_path = os.path.abspath(os.path.join(root, f))
                        if serve_mode:
                            # Use /api/file/ route with URL-encoded absolute path
                            files.append(f"/api/file/{full_path.replace(chr(92), '/')}")
                        else:
                            files.append(f"file:///{full_path.replace(chr(92), '/')}")
            if files:
                m[k] = sorted(files)
        return m

    def _scan_images(self, serve_mode: bool = False) -> Dict[str, Dict[str, Dict[str, str]]]:
        if not os.path.isdir(self.html_root):
            return {}
        
        sensor_names = {"range", "velocity", "rangerate", "doppler", "azimuth", "elevation", "snr", "rcs"}
        sensor_positions = {"FL", "FR", "RL", "RR", "FC"}
        img_map: Dict[str, Dict[str, Dict[str, str]]] = {}
        
        for entry in os.scandir(self.html_root):
            if not entry.is_dir():
                continue
            k = self._key(entry.name)
            if not k:
                continue
            
            imgs: Dict[str, Dict[str, str]] = {}
            for root, _, fnames in os.walk(entry.path):
                folder_name = os.path.basename(root).upper()
                for f in fnames:
                    if os.path.splitext(f)[1].lower() != ".png":
                        continue
                    
                    file_base = os.path.splitext(f)[0].lower()
                    sensor_pos = next((pos for pos in sensor_positions if folder_name == pos or pos.lower() in file_base), None)
                    if not sensor_pos:
                        continue
                    
                    sensor_name = next((sn for sn in sensor_names if sn in file_base), "other")
                    if sensor_name not in imgs:
                        imgs[sensor_name] = {}
                    
                    full_path = os.path.abspath(os.path.join(root, f))
                    if serve_mode:
                        imgs[sensor_name][sensor_pos] = f"/api/file/{full_path.replace(chr(92), '/')}"
                    else:
                        imgs[sensor_name][sensor_pos] = f"file:///{full_path.replace(chr(92), '/')}"
            
            if imgs:
                img_map[k] = imgs
        
        return img_map

    def _scan_text(self, serve_mode: bool = False) -> Dict[str, Dict[str, str]]:
        text_dir = os.path.join(self.video_root, "log_txt")
        if not os.path.isdir(text_dir):
            return {}
        
        m: Dict[str, Dict[str, str]] = {}
        for entry in os.scandir(text_dir):
            if not entry.is_file() or os.path.splitext(entry.name)[1].lower() != ".txt":
                continue
            k = self._key(entry.name)
            if not k:
                continue
            try:
                with open(entry.path, "r", encoding="utf-8") as f:
                    content = f.read()
            except Exception:
                content = ""
            full_path = os.path.abspath(entry.path)
            m[k] = {
                "path": full_path.replace("\\", "/"),  # Store absolute path
                "content": content
            }
        return m

    def _scan_video(self, serve_mode: bool = False) -> Dict[str, str]:
        if not os.path.isdir(self.video_root):
            return {}
        m: Dict[str, str] = {}
        for entry in os.scandir(self.video_root):
            if not entry.is_file() or os.path.splitext(entry.name)[1].lower() not in {".mp4", ".avi", ".mov", ".mkv"}:
                continue
            k = self._key(entry.name)
            if k:
                full_path = os.path.abspath(entry.path)
                if serve_mode:
                    # Use /api/file/ route with absolute path
                    m[k] = f"/api/file/{full_path.replace(chr(92), '/')}"
                else:
                    m[k] = f"file:///{full_path.replace(chr(92), '/')}"
        return m

    def _get_html_folder_name(self, key: str) -> str:
        for entry in os.scandir(self.html_root):
            if entry.is_dir() and self._key(entry.name) == key:
                return entry.name
        return key

    def run(self, serve_mode: bool = False) -> None:
        html_map = self._scan_html(serve_mode=serve_mode)
        img_map = self._scan_images(serve_mode=serve_mode)
        video_map = self._scan_video(serve_mode=serve_mode)
        text_map = self._scan_text(serve_mode=serve_mode)

        html_keys = set(html_map)
        video_keys = set(video_map)
        matched = sorted(html_keys & video_keys)

        for k in sorted(html_keys - video_keys):
            print(f"No video found for key: {k}")
        for k in sorted(video_keys - html_keys):
            print(f"No HTML folder found for key: {k}")

        if not matched:
            print("No matching HTML/video pairs found.")
            return

        mapping: Dict[str, Dict[str, Any]] = {}
        print("Matched keys:")
        for k in matched:
            video_url = video_map[k]
            video_path = video_url.replace("file:///", "")
            video_name = os.path.basename(video_path)
            
            text_info = text_map.get(k)
            if text_info:
                comment_path = text_info["path"]
                comment_content = text_info["content"]
            else:
                base_name = os.path.splitext(video_name)[0]
                comment_path = f"db/video/log_txt/{base_name}.txt"
                comment_content = ""
            
            comment_abs_path = os.path.abspath(os.path.join(self.output_dir, comment_path)).replace("\\", "/")
            
            mapping[k] = {
                "video": video_url,
                "video_name": video_name,
                "html_files": html_map[k],
                "html_folder": self._get_html_folder_name(k),
                "images": img_map.get(k, {}),
                "comment_path": comment_path,
                "comment_abs_path": comment_abs_path,
                "comment_content": comment_content,
            }
            print(f"  {k} -> {len(html_map[k])} HTML file(s), video: {video_url}")

        build_html(self.output_html, mapping, serve_mode=serve_mode)
        print(f"Viewer HTML generated at: {self.output_html}")
        
        # Update the latest viewer path so Flask serves the new file
        set_latest_viewer(self.output_html)


@app.after_request
def add_no_cache_headers(response):
    """Prevent browser caching to ensure fresh content"""
    response.headers['Cache-Control'] = 'no-cache, no-store, must-revalidate'
    response.headers['Pragma'] = 'no-cache'
    response.headers['Expires'] = '0'
    return response


@app.route('/')
def serve_viewer():
    """Serve the latest generated viewer.html"""
    # If a viewer was generated somewhere else, serve that first
    if LATEST_VIEWER_PATH and os.path.isfile(LATEST_VIEWER_PATH):
        dirp = os.path.dirname(LATEST_VIEWER_PATH)
        name = os.path.basename(LATEST_VIEWER_PATH)
        return send_from_directory(dirp, name)
    # Fallback to bundled out folder
    return send_from_directory(OUT_DIR, 'viewer.html')


@app.route('/out/<path:filename>')
def serve_out(filename):
    """Serve static files from out directory or latest viewer directory"""
    # Prefer file from OUT_DIR
    candidate = os.path.join(OUT_DIR, filename)
    if os.path.exists(candidate):
        return send_from_directory(OUT_DIR, filename)

    # Otherwise, if latest viewer lives in another out folder, serve from there
    if LATEST_VIEWER_PATH:
        alt_dir = os.path.dirname(LATEST_VIEWER_PATH)
        alt_candidate = os.path.join(alt_dir, filename)
        if os.path.exists(alt_candidate):
            return send_from_directory(alt_dir, filename)

    abort(404)


@app.route('/db/<path:filename>')
def serve_db(filename):
    """Serve files from db directory"""
    # First try the default DB_DIR
    candidate = os.path.join(DB_DIR, filename)
    if os.path.exists(candidate):
        return send_from_directory(DB_DIR, filename)
    
    # If viewer_app has a custom path, try that
    if viewer_app:
        # Try html_root parent
        html_parent = os.path.dirname(viewer_app.html_root)
        alt_candidate = os.path.join(html_parent, filename)
        if os.path.exists(alt_candidate):
            return send_from_directory(html_parent, filename)
        
        # Try video_root parent
        video_parent = os.path.dirname(viewer_app.video_root)
        alt_candidate = os.path.join(video_parent, filename)
        if os.path.exists(alt_candidate):
            return send_from_directory(video_parent, filename)
    
    abort(404)


@app.route('/api/file/<path:file_path>')
def serve_file_by_path(file_path):
    """Serve any file by its absolute path (Unix-style forward slashes)"""
    # Convert forward slashes back to OS path and handle Windows drive letters
    # The path comes in as: C:/Users/... or /home/...
    if len(file_path) >= 2 and file_path[1] == ':':
        # Windows path like C:/Users/...
        abs_path = file_path.replace('/', os.sep)
    else:
        # Unix path
        abs_path = '/' + file_path.replace('/', os.sep)
    
    if os.path.exists(abs_path) and os.path.isfile(abs_path):
        directory = os.path.dirname(abs_path)
        filename = os.path.basename(abs_path)
        return send_from_directory(directory, filename)
    
    abort(404)


# API: Get/Set latest viewer path
@app.route('/api/viewer/latest', methods=['GET'])
def api_latest_viewer():
    return jsonify({'path': LATEST_VIEWER_PATH})


@app.route('/api/viewer/set', methods=['POST'])
def api_set_viewer():
    payload = request.get_json(silent=True) or {}
    p = payload.get('path') or payload.get('viewer_path')
    if not p:
        return jsonify({'error': 'missing path'}), 400
    set_latest_viewer(p)
    return jsonify({'ok': True, 'path': LATEST_VIEWER_PATH})


@app.route('/api/save-comment', methods=['POST'])
def save_comment():
    try:
        data = request.get_json()
        file_path = data.get('path')
        content = data.get('content', '')
        
        if not file_path:
            return jsonify({'success': False, 'error': 'No path provided'}), 400
        
        # The path is now stored as absolute path (Unix-style forward slashes)
        # Convert to OS-specific path
        if '/' in file_path and (len(file_path) >= 2 and file_path[1] == ':'):
            # Windows absolute path like C:/Users/...
            abs_path = file_path.replace('/', os.sep)
        elif file_path.startswith('/') and os.name != 'nt':
            # Unix absolute path
            abs_path = file_path
        else:
            # Fallback: try relative to output_dir
            if viewer_app:
                abs_path = os.path.abspath(os.path.join(viewer_app.output_dir, file_path))
            else:
                abs_path = os.path.abspath(file_path)
        
        # Ensure the directory exists
        os.makedirs(os.path.dirname(abs_path), exist_ok=True)
        
        # Write the content
        with open(abs_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        return jsonify({'success': True, 'path': abs_path})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@app.route('/api/reload', methods=['POST'])
def reload_viewer():
    global viewer_app
    try:
        if viewer_app:
            viewer_app.run(serve_mode=True)
            return jsonify({'success': True})
        return jsonify({'success': False, 'error': 'No viewer app initialized'}), 500
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@app.route('/api/get-comment', methods=['POST'])
def get_comment():
    """Get latest content of a comment file"""
    try:
        data = request.get_json()
        file_path = data.get('path')
        
        if not file_path:
            return jsonify({'success': False, 'error': 'No path provided'}), 400
        
        # The path is now stored as absolute path (Unix-style forward slashes)
        # Convert to OS-specific path
        if '/' in file_path and (len(file_path) >= 2 and file_path[1] == ':'):
            # Windows absolute path like C:/Users/...
            abs_path = file_path.replace('/', os.sep)
        elif file_path.startswith('/') and os.name != 'nt':
            # Unix absolute path
            abs_path = file_path
        else:
            # Fallback: try relative to output_dir
            if viewer_app:
                abs_path = os.path.abspath(os.path.join(viewer_app.output_dir, file_path))
            else:
                abs_path = os.path.abspath(file_path)
        
        if not os.path.exists(abs_path):
            return jsonify({'success': True, 'content': ''})
        
        with open(abs_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        return jsonify({'success': True, 'content': content})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@app.route('/cluster')
def cluster_page():
    return send_from_directory(OUT_DIR, 'cluster.html')


@app.route('/api/cluster/servers', methods=['GET'])
def get_servers():
    return jsonify({'servers': list(SERVERS.keys())})


@app.route('/api/cluster/connect', methods=['POST'])
def cluster_connect_api():
    try:
        data = request.get_json()
        server = data.get('server', '')
        username = data.get('username', '')
        password = data.get('password', '')
        
        if not all([server, username, password]):
            return jsonify({'success': False, 'error': 'Missing credentials'}), 400
        
        success, msg = cluster.connect(server, username, password)
        return jsonify({'success': success, 'message': msg})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@app.route('/api/cluster/disconnect', methods=['POST'])
def cluster_disconnect():
    cluster.disconnect()
    return jsonify({'success': True})


@app.route('/api/cluster/status', methods=['GET'])
def cluster_status():
    return jsonify({
        'connected': cluster.connected,
        'server': cluster.server_name if cluster.connected else None
    })


@app.route('/api/cluster/list', methods=['POST'])
def cluster_list():
    try:
        data = request.get_json()
        path = data.get('path', '/')
        success, items = cluster.list_dir(path)
        return jsonify({'success': success, 'items': items, 'path': path})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500


@app.route('/api/cluster/download', methods=['POST'])
def cluster_download():
    global viewer_app
    try:
        data = request.get_json()
        html_path = data.get('html_path', '')
        video_path = data.get('video_path', '')
        local_save_path = data.get('local_path', 'db').strip() or 'db'
        
        if not html_path or not video_path:
            return jsonify({'success': False, 'error': 'Both remote paths required'}), 400
        
        # Reset progress tracking
        reset_progress()
        download_progress["active"] = True
        
        # Use the user-specified local path (make it absolute)
        local_save_path = os.path.abspath(local_save_path)
        local_html = os.path.join(local_save_path, 'html')
        local_video = os.path.join(local_save_path, 'video')
        
        # Determine output path for viewer.html
        output_html = os.path.join(OUT_DIR, 'viewer.html')
        
        # Download HTML folder
        update_progress(phase="downloading", message=f"Downloading HTML from {html_path}...")
        success1, msg1 = cluster.download_directory(html_path, local_html)
        if not success1:
            download_progress["active"] = False
            update_progress(phase="error", message=f'HTML download failed: {msg1}')
            return jsonify({'success': False, 'error': f'HTML download failed: {msg1}'})
        
        # Download video folder
        update_progress(phase="downloading", message=f"Downloading video from {video_path}...")
        success2, msg2 = cluster.download_directory(video_path, local_video)
        if not success2:
            download_progress["active"] = False
            update_progress(phase="error", message=f'Video download failed: {msg2}')
            return jsonify({'success': False, 'error': f'Video download failed: {msg2}'})
        
        # Update viewer with new paths
        update_progress(phase="processing", message="Building viewer HTML...")
        viewer_app = LogViewerApp(local_html, local_video, output_html)
        viewer_app.run(serve_mode=True)
        
        # set_latest_viewer is now called inside viewer_app.run()
        
        download_progress["active"] = False
        update_progress(phase="complete", message=f'{msg1}, {msg2}')
        
        return jsonify({
            'success': True,
            'message': f'{msg1}, {msg2}',
            'html_local': local_html,
            'video_local': local_video,
            'viewer_path': output_html
        })
    except Exception as e:
        import traceback
        download_progress["active"] = False
        update_progress(phase="error", message=str(e))
        return jsonify({'success': False, 'error': f'{str(e)}: {traceback.format_exc()}'}), 500


@app.route('/api/cluster/progress', methods=['GET'])
def cluster_progress():
    """Return current download progress"""
    return jsonify(download_progress)


def main() -> None:
    global viewer_app
    serve_mode = '--serve' in sys.argv
    vlm_mode = '--vlm' in sys.argv
    force_vlm = '--force' in sys.argv
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    
    if vlm_mode:
        # VLM mode - process videos and generate text descriptions
        from vlm import process_videos_with_vlm
        
        # Get video root from args or use default
        video_root = os.path.abspath(args[0]) if len(args) >= 1 else os.path.join(BASE_DIR, 'db', 'video')
        
        if not os.path.isdir(video_root):
            print(f"Error: Video directory not found: {video_root}", file=sys.stderr)
            sys.exit(1)
        
        print(f"\n{'='*60}")
        print("VLM Mode: Generating text descriptions for videos")
        print(f"Video folder: {video_root}")
        print(f"Force regenerate: {force_vlm}")
        print(f"{'='*60}")
        
        processed, skipped, errors = process_videos_with_vlm(video_root, force=force_vlm)
        
        if errors > 0:
            sys.exit(1)
        sys.exit(0)
    
    if serve_mode:
        # Server mode - paths are optional, use defaults if not provided
        html_root = os.path.abspath(args[0]) if len(args) >= 1 else os.path.join(BASE_DIR, 'db', 'html')
        video_root = os.path.abspath(args[1]) if len(args) >= 2 else os.path.join(BASE_DIR, 'db', 'video')

        if len(args) >= 3:
            output_html = os.path.abspath(args[2])
        else:
            output_html = os.path.join(OUT_DIR, 'viewer.html')

        # Check if default paths exist, create empty viewer if not
        if os.path.isdir(html_root) and os.path.isdir(video_root):
            viewer_app = LogViewerApp(html_root, video_root, output_html)
            viewer_app.run(serve_mode=True)
        else:
            # Create empty viewer for online-only mode
            os.makedirs(OUT_DIR, exist_ok=True)
            build_html(output_html, {}, serve_mode=True)
            set_latest_viewer(output_html)
            print("No local data found. Use 'Go Online' to connect to cluster.")
        
        port = 5000
        print(f"\nStarting Flask server at http://localhost:{port}")
        print("Press Ctrl+C to stop the server")
        
        def open_browser():
            import time
            time.sleep(1)
            webbrowser.open(f'http://localhost:{port}')
        
        threading.Thread(target=open_browser, daemon=True).start()
        app.run(host='0.0.0.0', port=port, debug=False)
    else:
        # Offline mode - paths are required
        if len(args) < 2:
            print("Usage:", file=sys.stderr)
            print("  Server mode:  python main.py --serve [html_root] [video_root] [output_html]", file=sys.stderr)
            print("  Offline mode: python main.py <html_root> <video_root> [output_html]", file=sys.stderr)
            print("  VLM mode:     python main.py --vlm [video_root] [--force]", file=sys.stderr)
            print("", file=sys.stderr)
            print("Options:", file=sys.stderr)
            print("  --serve   Start Flask server for interactive viewing", file=sys.stderr)
            print("  --vlm     Generate text descriptions for videos using VLM", file=sys.stderr)
            print("  --force   (with --vlm) Regenerate text even if .txt file exists", file=sys.stderr)
            sys.exit(1)
        
        html_root = args[0]
        video_root = args[1]
        output_html = args[2] if len(args) >= 3 else 'out/viewer.html'
        
        viewer_app = LogViewerApp(html_root, video_root, output_html)
        viewer_app.run(serve_mode=False)
        print(f"\nViewer generated. Open {output_html} in a browser.")


if __name__ == "__main__":
    main()