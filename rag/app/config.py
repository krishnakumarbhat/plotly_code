from __future__ import annotations

import os
from pathlib import Path

from dotenv import load_dotenv


class AppConfig:
    def __init__(self) -> None:
        self.project_root = Path(__file__).resolve().parents[1]
        # Ensure local .env values are available when running via `python main.py`.
        load_dotenv(self.project_root / ".env")
        self.data_dir = self.project_root / "data"
        self.sqlite_path = self.data_dir / "rag_logs.db"
        self.chroma_dir = self.data_dir / "chroma"
        self.vector_store_dir = self.data_dir / "vector_store"

        raw_html_root = os.getenv(
            "HTML_ROOT_PATH",
            r"C:\Users\ouymc2\Desktop\plotly_code\main_html\all_services\tools\intplot_kpi",
        )
        self.html_root_path = Path(self._sanitize_path(raw_html_root))
        self.file_keyword = os.getenv(
            "HTML_FILE_KEYWORD",
            "sil_narrative,sil_nartive,_sil_narrative,_sil_nartive",
        )
        self.exclude_file_keyword = os.getenv("HTML_EXCLUDE_FILE_KEYWORD", "dummy")
        self.allow_all_html_fallback = os.getenv("ALLOW_ALL_HTML_FALLBACK", "false").lower() in {
            "1",
            "true",
            "yes",
            "y",
        }
        self.max_files_per_ingest = int(os.getenv("MAX_FILES_PER_INGEST", "0"))
        self.parse_english_only = os.getenv("PARSE_ENGLISH_ONLY", "true").lower() in {
            "1",
            "true",
            "yes",
            "y",
        }

        default_embed_path = self.project_root / "model" / "nomic-embed-text-v1.5"
        self.embedding_model_path = Path(
            self._sanitize_path(os.getenv("EMBED_MODEL_PATH", str(default_embed_path)))
        )
        default_gguf = self.project_root / "model" / "Qwen_Qwen3.5-2B-Q5_K_S.gguf"
        self.qwen_gguf_path = Path(
            self._sanitize_path(os.getenv("QWEN_GGUF_PATH", str(default_gguf)))
        )
        default_fallback_gguf = self.project_root / "model" / "Qwen_Qwen3.5-2B-Q5_K_S.gguf"
        self.qwen_fallback_gguf_path = Path(
            self._sanitize_path(
                os.getenv("QWEN_FALLBACK_GGUF_PATH", str(default_fallback_gguf))
            )
        )
        self.qwen_model_id = os.getenv("QWEN_MODEL_ID", "")
        self.llm_backend = os.getenv("LLM_BACKEND", "llama_server").strip().lower()
        if self.llm_backend not in {"llama_server", "llama_cpp_python"}:
            self.llm_backend = "llama_server"
        default_llama_server_path = self.project_root / "tools" / "llama.cpp-vulkan" / "llama-server.exe"
        if not default_llama_server_path.exists():
            default_llama_server_path = self.project_root / "tools" / "llama.cpp" / "llama-server.exe"
        self.llama_server_path = Path(
            self._sanitize_path(os.getenv("LLAMA_SERVER_PATH", str(default_llama_server_path)))
        )
        self.llama_server_host = os.getenv("LLAMA_SERVER_HOST", "127.0.0.1")
        self.llama_server_port = int(os.getenv("LLAMA_SERVER_PORT", "8081"))
        self.llama_server_base_url = os.getenv(
            "LLAMA_SERVER_BASE_URL",
            f"http://{self.llama_server_host}:{self.llama_server_port}",
        )
        self.llama_server_autostart = os.getenv("LLAMA_SERVER_AUTOSTART", "true").lower() in {
            "1",
            "true",
            "yes",
            "y",
        }
        self.llm_max_new_tokens = int(os.getenv("LLM_MAX_NEW_TOKENS", "640"))
        self.llm_context_window = int(os.getenv("LLM_CONTEXT_WINDOW", "4096"))
        self.llm_similarity_top_k = int(os.getenv("LLM_SIMILARITY_TOP_K", "6"))
        self.llm_n_threads = int(os.getenv("LLM_N_THREADS", str(os.cpu_count() or 8)))
        self.llm_n_batch = int(os.getenv("LLM_N_BATCH", "512"))
        self.llm_n_gpu_layers = int(os.getenv("LLM_N_GPU_LAYERS", "24"))
        self.chunk_size = int(os.getenv("CHUNK_SIZE", "2000"))
        self.chunk_overlap = int(os.getenv("CHUNK_OVERLAP", "80"))
        self.vector_backend = os.getenv("VECTOR_BACKEND", "simple").strip().lower()
        if self.vector_backend not in {"simple", "chroma"}:
            self.vector_backend = "simple"
        default_vector_store_path = self.vector_store_dir / "vector_store.json"
        self.vector_store_json_path = Path(
            self._sanitize_path(
                os.getenv("VECTOR_STORE_JSON_PATH", str(default_vector_store_path))
            )
        )
        self.collection_name = os.getenv("CHROMA_COLLECTION", "html_narrative_docs")
        self.host = os.getenv("FLASK_HOST", "127.0.0.1")
        self.port = int(os.getenv("FLASK_PORT", "5000"))

        self.data_dir.mkdir(parents=True, exist_ok=True)
        self.chroma_dir.mkdir(parents=True, exist_ok=True)
        self.vector_store_dir.mkdir(parents=True, exist_ok=True)

    @staticmethod
    def _sanitize_path(raw_path: str) -> str:
        cleaned = raw_path.strip().strip('"').strip("'")
        if cleaned.endswith("'"):
            cleaned = cleaned[:-1]
        return cleaned
