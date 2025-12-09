import os
import sys
import threading
import webbrowser
from typing import Any, Dict, List, Optional
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
from html_build import build_html
from cluster_connect import cluster, SERVERS, download_progress, reset_progress, update_progress

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(BASE_DIR, "out")
app = Flask(__name__, static_folder=OUT_DIR)
DB_DIR = os.path.join(BASE_DIR, "db")


CORS(app)

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
            files = [
                os.path.relpath(os.path.join(root, f), self.output_dir).replace("\\", "/")
                for root, _, fnames in os.walk(entry.path)
                for f in fnames
                if os.path.splitext(f)[1].lower() in {".html", ".htm"}
            ]
            if files:
                m[k] = sorted(files)
        return m

    def _scan_images(self) -> Dict[str, Dict[str, Dict[str, str]]]:
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
                    
                    imgs[sensor_name][sensor_pos] = os.path.relpath(os.path.join(root, f), self.output_dir).replace("\\", "/")
            
            if imgs:
                img_map[k] = imgs
        
        return img_map

    def _scan_text(self) -> Dict[str, Dict[str, str]]:
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
            m[k] = {
                "path": os.path.relpath(entry.path, self.output_dir).replace("\\", "/"),
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
                if serve_mode:
                    # Use relative path for Flask serving
                    rel_path = os.path.relpath(entry.path, self.output_dir).replace("\\", "/")
                    m[k] = rel_path
                else:
                    m[k] = f"file:///{os.path.abspath(entry.path).replace(chr(92), '/')}"
        return m

    def _get_html_folder_name(self, key: str) -> str:
        for entry in os.scandir(self.html_root):
            if entry.is_dir() and self._key(entry.name) == key:
                return entry.name
        return key

    def run(self, serve_mode: bool = False) -> None:
        html_map = self._scan_html()
        img_map = self._scan_images()
        video_map = self._scan_video(serve_mode=serve_mode)
        text_map = self._scan_text()

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


viewer_app = None

@app.after_request
def add_no_cache_headers(response):
    """Prevent browser caching to ensure fresh content"""
    response.headers['Cache-Control'] = 'no-cache, no-store, must-revalidate'
    response.headers['Pragma'] = 'no-cache'
    response.headers['Expires'] = '0'
    return response

@app.route('/')
def serve_viewer():
    return send_from_directory(OUT_DIR, 'viewer.html')

@app.route('/out/<path:filename>')
def serve_static(filename):
    return send_from_directory(OUT_DIR, filename)

@app.route('/db/<path:filename>')
def serve_db(filename):
    return send_from_directory(DB_DIR, filename)

@app.route('/api/save-comment', methods=['POST'])
def save_comment():
    try:
        data = request.get_json()
        file_path = data.get('path')
        content = data.get('content', '')
        
        if not file_path:
            return jsonify({'success': False, 'error': 'No path provided'}), 400
        
        # Resolve the path relative to the output directory
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
        
        # Resolve the path relative to the output directory
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
def cluster_connect():
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
        
        # Use the user-specified local path
        local_html = os.path.abspath(os.path.join(local_save_path, 'html'))
        local_video = os.path.abspath(os.path.join(local_save_path, 'video'))
        
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
        viewer_app = LogViewerApp(local_html, local_video, 'out/viewer.html')
        viewer_app.run(serve_mode=True)
        
        download_progress["active"] = False
        update_progress(phase="complete", message=f'{msg1}, {msg2}')
        
        return jsonify({
            'success': True,
            'message': f'{msg1}, {msg2}',
            'html_local': local_html,
            'video_local': local_video
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
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    
    if serve_mode:
        # Server mode - paths are optional, use defaults if not provided
        html_root = args[0] if len(args) >= 1 else 'db/html'
        video_root = args[1] if len(args) >= 2 else 'db/video'

        if len(args) >= 3:
            # If user gave a path, resolve it relative to current working dir
            output_html = os.path.abspath(args[2])
        else:
            # Default to out/viewer.html under BASE_DIR
            output_html = os.path.join(OUT_DIR, 'viewer.html')

        # Check if default paths exist, create empty viewer if not
        if os.path.isdir(html_root) and os.path.isdir(video_root):
            viewer_app = LogViewerApp(html_root, video_root, output_html)
            viewer_app.run(serve_mode=True)
        else:
            # Create empty viewer for online-only mode
            os.makedirs('out', exist_ok=True)
            from html_build import build_html
            build_html(output_html, {}, serve_mode=True)
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
            sys.exit(1)
        
        html_root = args[0]
        video_root = args[1]
        output_html = args[2] if len(args) >= 3 else 'out/viewer.html'
        
        viewer_app = LogViewerApp(html_root, video_root, output_html)
        viewer_app.run(serve_mode=False)
        print(f"\nViewer generated. Open {output_html} in a browser.")


if __name__ == "__main__":
    main()