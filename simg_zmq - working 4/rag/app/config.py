from __future__ import annotations

import os
from pathlib import Path


class AppConfig:
    def __init__(self) -> None:
        self.project_root = Path(__file__).resolve().parents[1]
        self.workspace_root = self.project_root.parent
        default_data_dir = Path('/tmp/rag_data') if self._running_in_container() else self.project_root / 'data'
        self.data_dir = Path(self._sanitize_path(os.getenv('RAG_DATA_DIR', str(default_data_dir))))
        self.sqlite_path = Path(
            self._sanitize_path(os.getenv('RAG_SQLITE_PATH', str(self.data_dir / 'rag_logs.db')))
        )
        self.vector_store_dir = Path(
            self._sanitize_path(
                os.getenv('RAG_VECTOR_STORE_DIR', str(self.data_dir / 'vector_store'))
            )
        )

        raw_html_root = os.getenv(
            "HTML_ROOT_PATH",
            str(self.workspace_root),
        )
        self.html_root_path = Path(self._sanitize_path(raw_html_root))
        self.file_keyword = os.getenv("HTML_FILE_KEYWORD", "")
        self.exclude_file_keyword = os.getenv("HTML_EXCLUDE_FILE_KEYWORD", "dummy")
        self.allow_all_html_fallback = os.getenv("ALLOW_ALL_HTML_FALLBACK", "true").lower() in {
            "1",
            "true",
            "yes",
            "y",
        }
        self.max_files_per_ingest = int(os.getenv("MAX_FILES_PER_INGEST", "0"))
        self.max_file_bytes = int(os.getenv("MAX_FILE_BYTES", str(2 * 1024 * 1024)))
        self.parse_english_only = os.getenv("PARSE_ENGLISH_ONLY", "true").lower() in {
            "1",
            "true",
            "yes",
            "y",
        }
        self.supported_extensions = [
            extension.strip().lower()
            for extension in os.getenv(
                "RAG_SUPPORTED_EXTENSIONS",
                ".html,.htm,.md,.txt,.json,.xml,.yaml,.yml,.py",
            ).split(",")
            if extension.strip()
        ]
        self.exclude_dirs = {
            directory.strip().lower()
            for directory in os.getenv(
                "RAG_EXCLUDE_DIRS",
                ".git,__pycache__,.pytest_cache,.venv,venv,node_modules,model,llm_model,data,simg_sh_hpcc",
            ).split(",")
            if directory.strip()
        }
        self.chunk_size = int(os.getenv("CHUNK_SIZE", "1800"))
        self.chunk_overlap = int(os.getenv("CHUNK_OVERLAP", "160"))
        self.vector_backend = "simple"
        default_vector_store_path = self.vector_store_dir / "vector_store.json"
        self.vector_store_json_path = Path(
            self._sanitize_path(
                os.getenv("VECTOR_STORE_JSON_PATH", str(default_vector_store_path))
            )
        )
        self.collection_name = os.getenv("CHROMA_COLLECTION", "html_narrative_docs")
        self.llm_similarity_top_k = int(os.getenv("LLM_SIMILARITY_TOP_K", "5"))
        self.auto_ingest_on_start = os.getenv("RAG_AUTO_INGEST_ON_START", "true").lower() in {
            "1",
            "true",
            "yes",
            "y",
        }
        self.host = os.getenv("FLASK_HOST", "127.0.0.1")
        self.port = int(os.getenv("FLASK_PORT", "5000"))

        self.data_dir.mkdir(parents=True, exist_ok=True)
        self.sqlite_path.parent.mkdir(parents=True, exist_ok=True)
        self.vector_store_dir.mkdir(parents=True, exist_ok=True)

    @staticmethod
    def _running_in_container() -> bool:
        return (
            Path('/.singularity.d').exists()
            or Path('/.dockerenv').exists()
            or bool(os.getenv('SINGULARITY_CONTAINER'))
            or bool(os.getenv('DOCKER_CONTAINER'))
        )

    @staticmethod
    def _sanitize_path(raw_path: str) -> str:
        cleaned = raw_path.strip().strip('"').strip("'")
        if cleaned.endswith("'"):
            cleaned = cleaned[:-1]
        return cleaned
