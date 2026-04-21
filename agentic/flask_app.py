import argparse
import os

from flask import Flask, jsonify, render_template, request

from work_agent.chat_service import WorkChatService
from work_agent.config import WorkConfig


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Work Flask Chat UI")
    parser.add_argument("--model", default="gpt-oss-20b-F16.gguf", help="Path to GGUF model")
    parser.add_argument("--docs", default="docs", help="Docs folder path")
    parser.add_argument("--query", default="Hello", help="Not used by web route; retained for config compatibility")

    parser.add_argument("--chroma-path", default="chroma_db", help="Chroma persistence path")
    parser.add_argument("--collection", default="work_knowledge", help="Chroma collection name")
    parser.add_argument("--top-k", type=int, default=3, help="Retrieved chunks count")

    parser.add_argument("--n-ctx", type=int, default=1024, help="Model context window")
    parser.add_argument("--threads", type=int, default=max((os.cpu_count() or 4) - 1, 1), help="CPU threads")
    parser.add_argument("--max-tokens", type=int, default=128, help="Max output tokens")
    parser.add_argument("--n-gpu-layers", type=int, default=0, help="GPU offload layers (0=CPU/system RAM)")
    parser.add_argument("--no-mmap", action="store_true", help="Disable memory-mapped model loading")
    parser.add_argument("--mlock", action="store_true", help="Lock model pages in RAM")

    parser.add_argument("--rebuild", action="store_true", help="Rebuild index from docs")
    parser.add_argument("--host", default="127.0.0.1", help="Flask host")
    parser.add_argument("--port", type=int, default=5000, help="Flask port")
    parser.add_argument("--debug", action="store_true", help="Enable Flask debug mode")
    return parser


def create_app(chat_service: WorkChatService) -> Flask:
    app = Flask(__name__, template_folder="templates")

    @app.get("/")
    def index():
        return render_template("index.html")

    @app.get("/api/health")
    def health():
        return jsonify(chat_service.health())

    @app.post("/api/chat")
    def chat():
        payload = request.get_json(silent=True) or {}
        query = (payload.get("message") or "").strip()
        if not query:
            return jsonify({"error": "message is required"}), 400

        result = chat_service.ask(query)
        return jsonify(result)

    return app


def main():
    args = build_parser().parse_args()
    config = WorkConfig.from_args(args)
    chat_service = WorkChatService(config)
    app = create_app(chat_service)

    print("[Work] Flask UI ready", flush=True)
    print(f"[Work] Open http://{args.host}:{args.port}", flush=True)
    app.run(host=args.host, port=args.port, debug=args.debug)


if __name__ == "__main__":
    main()
