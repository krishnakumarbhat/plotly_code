from __future__ import annotations

from app.api import RagApi
from app.config import AppConfig
from app.graph_flow import QueryGraph
from app.html_parser import HtmlParser
from app.ingestor import HtmlIngestor
from app.rag_engine import RagEngine
from app.sqlite_store import SQLiteLogStore


class Application:
    def __init__(self, html_root_override: str | None = None) -> None:
        self.config = AppConfig()
        # allow overriding the HTML root via CLI
        if html_root_override:
            from pathlib import Path

            self.config.html_root_path = Path(html_root_override)

        self.sqlite_store = SQLiteLogStore(self.config.sqlite_path)
        self.rag_engine = RagEngine(
            chroma_path="",
            collection_name=self.config.collection_name,
            vector_backend=self.config.vector_backend,
            vector_store_json_path=str(self.config.vector_store_json_path),
            embedding_model_path="",
            qwen_gguf_path="",
            qwen_fallback_gguf_path="",
            qwen_model_id="",
            llm_backend="simple",
            llama_server_path="",
            llama_server_base_url="",
            llama_server_host="",
            llama_server_port=0,
            llama_server_autostart=False,
            llm_context_window=0,
            llm_max_new_tokens=0,
            llm_similarity_top_k=self.config.llm_similarity_top_k,
            llm_n_threads=0,
            llm_n_batch=0,
            llm_n_gpu_layers=0,
            chunk_size=self.config.chunk_size,
            chunk_overlap=self.config.chunk_overlap,
        )
        self.parser = HtmlParser(english_only=self.config.parse_english_only)
        self.ingestor = HtmlIngestor(
            html_root=self.config.html_root_path,
            file_keyword=self.config.file_keyword,
            exclude_file_keyword=self.config.exclude_file_keyword,
            allow_all_html_fallback=self.config.allow_all_html_fallback,
            max_files_per_ingest=self.config.max_files_per_ingest,
            max_file_bytes=self.config.max_file_bytes,
            supported_extensions=self.config.supported_extensions,
            exclude_dirs=self.config.exclude_dirs,
            parser=self.parser,
            rag_engine=self.rag_engine,
            sqlite_store=self.sqlite_store,
        )
        self.query_graph = QueryGraph(
            rag_engine=self.rag_engine,
            sqlite_store=self.sqlite_store,
        )
        self.api = RagApi(
            ingestor=self.ingestor,
            query_graph=self.query_graph,
            sqlite_store=self.sqlite_store,
        )
        self._bootstrap_index()

    def _bootstrap_index(self) -> None:
        if not self.config.auto_ingest_on_start:
            return
        if self.rag_engine.document_count > 0:
            return
        if not self.config.html_root_path.exists():
            return
        self.ingestor.ingest(run_vector=False)

    def run(self) -> None:
        self.api.app.run(host=self.config.host, port=self.config.port)
