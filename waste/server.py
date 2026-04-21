"""
Simple HTTP server for Log Viewer that allows reading and writing text files.
Run this in the background, then open the viewer HTML in a browser.
"""

import os
import sys
import json
import http.server
import socketserver
from urllib.parse import urlparse, parse_qs, unquote
from typing import Any
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS


class LogViewerHandler(http.server.SimpleHTTPRequestHandler):
    """Custom handler that adds API endpoints for file operations."""
    
    # Base directory for allowed file operations (security)
    ALLOWED_BASE = os.path.dirname(os.path.abspath(__file__))
    
    def do_GET(self):
        """Handle GET requests - serve files or read text content."""
        parsed = urlparse(self.path)
        
        if parsed.path == '/api/read':
            self._handle_read(parsed)
        else:
            # Serve static files normally
            super().do_GET()
    
    def do_POST(self):
        """Handle POST requests - save text content."""
        parsed = urlparse(self.path)
        
        if parsed.path == '/api/save':
            self._handle_save()
        else:
            self.send_error(404, "Not Found")
    
    def do_OPTIONS(self):
        """Handle CORS preflight requests."""
        self.send_response(200)
        self._send_cors_headers()
        self.end_headers()
    
    def _send_cors_headers(self):
        """Add CORS headers to allow browser requests."""
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
    
    def _is_path_allowed(self, filepath: str) -> bool:
        """Check if the file path is within allowed directory."""
        abs_path = os.path.abspath(filepath)
        return abs_path.startswith(self.ALLOWED_BASE)
    
    def _handle_read(self, parsed):
        """Read and return text file content."""
        params = parse_qs(parsed.query)
        file_path = params.get('path', [None])[0]
        
        if not file_path:
            self._send_json_response(400, {"error": "Missing 'path' parameter"})
            return
        
        file_path = unquote(file_path)
        
        # Convert relative path to absolute
        if not os.path.isabs(file_path):
            file_path = os.path.join(self.ALLOWED_BASE, file_path)
        
        if not self._is_path_allowed(file_path):
            self._send_json_response(403, {"error": "Access denied"})
            return
        
        if not os.path.exists(file_path):
            self._send_json_response(404, {"error": "File not found", "path": file_path})
            return
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            self._send_json_response(200, {"content": content, "path": file_path})
        except Exception as e:
            self._send_json_response(500, {"error": str(e)})
    
    def _handle_save(self):
        """Save content to a text file."""
        try:
            content_length = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(content_length).decode('utf-8')
            data = json.loads(body)
        except Exception as e:
            self._send_json_response(400, {"error": f"Invalid request body: {e}"})
            return
        
        file_path = data.get('path')
        content = data.get('content', '')
        
        if not file_path:
            self._send_json_response(400, {"error": "Missing 'path' in request body"})
            return
        
        # Convert relative path to absolute
        if not os.path.isabs(file_path):
            file_path = os.path.join(self.ALLOWED_BASE, file_path)
        
        if not self._is_path_allowed(file_path):
            self._send_json_response(403, {"error": "Access denied"})
            return
        
        try:
            # Create directory if it doesn't exist
            os.makedirs(os.path.dirname(file_path), exist_ok=True)
            
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            
            self._send_json_response(200, {"success": True, "path": file_path})
        except Exception as e:
            self._send_json_response(500, {"error": str(e)})
    
    def _send_json_response(self, status_code: int, data: Any):
        """Send a JSON response."""
        self.send_response(status_code)
        self.send_header('Content-Type', 'application/json')
        self._send_cors_headers()
        self.end_headers()
        self.wfile.write(json.dumps(data).encode('utf-8'))
    
    def log_message(self, format, *args):
        """Custom log format."""
        print(f"[Server] {args[0]}")


app = Flask(__name__)
CORS(app)

# Base directory for file operations (set when running)
BASE_DIR = os.getcwd()


def set_base_dir(path: str) -> None:
    global BASE_DIR
    BASE_DIR = os.path.abspath(path)


@app.route("/")
def index():
    """Serve the viewer.html file."""
    return send_from_directory(BASE_DIR, "viewer.html")


@app.route("/<path:filename>")
def serve_file(filename):
    """Serve static files (html, video, images, etc.)."""
    return send_from_directory(BASE_DIR, filename)


@app.route("/api/read", methods=["GET"])
def read_file():
    """Read content of a text file."""
    rel_path = request.args.get("path", "")
    if not rel_path:
        return jsonify({"error": "No path provided"}), 400

    full_path = os.path.normpath(os.path.join(BASE_DIR, rel_path))
    
    # Security check: ensure path is within BASE_DIR
    if not full_path.startswith(os.path.normpath(BASE_DIR)):
        return jsonify({"error": "Invalid path"}), 403

    if not os.path.isfile(full_path):
        return jsonify({"error": "File not found", "content": ""}), 404

    try:
        with open(full_path, "r", encoding="utf-8") as f:
            content = f.read()
        return jsonify({"content": content, "path": rel_path})
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/api/save", methods=["POST"])
def save_file():
    """Save content to a text file."""
    data = request.get_json()
    if not data:
        return jsonify({"error": "No JSON data provided"}), 400

    rel_path = data.get("path", "")
    content = data.get("content", "")

    if not rel_path:
        return jsonify({"error": "No path provided"}), 400

    full_path = os.path.normpath(os.path.join(BASE_DIR, rel_path))
    
    # Security check: ensure path is within BASE_DIR
    if not full_path.startswith(os.path.normpath(BASE_DIR)):
        return jsonify({"error": "Invalid path"}), 403

    try:
        # Create directory if it doesn't exist
        os.makedirs(os.path.dirname(full_path), exist_ok=True)
        
        with open(full_path, "w", encoding="utf-8") as f:
            f.write(content)
        return jsonify({"success": True, "path": rel_path})
    except Exception as e:
        return jsonify({"error": str(e)}), 500


def run_server(base_dir: str, port: int = 5000):
    """Run the Flask server."""
    set_base_dir(base_dir)
    print(f"Starting server at http://localhost:{port}")
    print(f"Serving files from: {BASE_DIR}")
    app.run(host="0.0.0.0", port=port, debug=False)


if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    run_server(".", port)
