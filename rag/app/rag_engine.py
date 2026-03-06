from __future__ import annotations

import io
import json
import logging
import os
import re
import subprocess
import time
import warnings
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from typing import Iterable
from urllib import error as urllib_error
from urllib import request as urllib_request

from llama_index.core import (
    Document,
    Settings,
    StorageContext,
    VectorStoreIndex,
    load_index_from_storage,
)
from llama_index.core.node_parser import SentenceSplitter
from llama_index.core.vector_stores import SimpleVectorStore
from llama_index.embeddings.huggingface import HuggingFaceEmbedding

warnings.filterwarnings(
    "ignore",
    message=r"The 'validate_default' attribute with value True was provided to the `Field\(\)` function.*",
)


class RagEngine:
    def __init__(
        self,
        chroma_path: str,
        collection_name: str,
        vector_backend: str,
        vector_store_json_path: str,
        embedding_model_path: str,
        qwen_gguf_path: str,
        qwen_fallback_gguf_path: str,
        qwen_model_id: str,
        llm_backend: str,
        llama_server_path: str,
        llama_server_base_url: str,
        llama_server_host: str,
        llama_server_port: int,
        llama_server_autostart: bool,
        llm_context_window: int,
        llm_max_new_tokens: int,
        llm_similarity_top_k: int,
        llm_n_threads: int,
        llm_n_batch: int,
        llm_n_gpu_layers: int,
        chunk_size: int,
        chunk_overlap: int,
    ) -> None:
        self._vector_backend = vector_backend
        self._vector_store_json_path = Path(vector_store_json_path)
        self._chroma_path = chroma_path
        self._db = None
        self._collection_name = collection_name
        self._collection = None
        self._vector_store = None
        self._storage_context: StorageContext | None = None

        self._embedding_model_path = embedding_model_path
        self._qwen_gguf_path = qwen_gguf_path
        self._qwen_fallback_gguf_path = qwen_fallback_gguf_path
        self._qwen_model_id = qwen_model_id
        self._llm_backend = llm_backend
        self._llama_server_path = Path(llama_server_path)
        self._llama_server_base_url = llama_server_base_url.rstrip("/")
        self._llama_server_host = llama_server_host
        self._llama_server_port = llama_server_port
        self._llama_server_autostart = llama_server_autostart
        self._llama_server_process: subprocess.Popen | None = None
        self._llm_context_window = llm_context_window
        self._llm_max_new_tokens = llm_max_new_tokens
        self._llm_similarity_top_k = llm_similarity_top_k
        self._llm_n_threads = llm_n_threads
        self._llm_n_batch = llm_n_batch
        self._llm_n_gpu_layers = llm_n_gpu_layers

        self._splitter = SentenceSplitter(chunk_size=chunk_size, chunk_overlap=chunk_overlap)
        self._index: VectorStoreIndex | None = None
        self._embedding_runtime_initialized = False
        self._llm_runtime_initialized = False
        self._initialize_vector_store()

    def _initialize_vector_store(self) -> None:
        if self._vector_backend == "chroma":
            self._initialize_chroma_vector_store()
            return
        self._initialize_simple_vector_store()

    def _initialize_simple_vector_store(self) -> None:
        self._vector_store_json_path.parent.mkdir(parents=True, exist_ok=True)
        self._vector_store = SimpleVectorStore()
        self._storage_context = StorageContext.from_defaults(vector_store=self._vector_store)

    def _initialize_chroma_vector_store(self) -> None:
        import chromadb
        from chromadb.config import Settings as ChromaSettings
        from llama_index.vector_stores.chroma import ChromaVectorStore

        self._db = chromadb.PersistentClient(
            path=self._chroma_path,
            settings=ChromaSettings(anonymized_telemetry=False),
        )
        self._collection = self._db.get_or_create_collection(self._collection_name)
        self._vector_store = ChromaVectorStore(chroma_collection=self._collection)

    def _persist_vector_store(self) -> None:
        if self._vector_backend != "simple":
            return
        self._index.storage_context.persist(
            persist_dir=str(self._vector_store_json_path.parent)
        )

    @staticmethod
    def _build_conversational_query(
        question: str,
        history: list[dict[str, str]] | None,
    ) -> str:
        if not history:
            return question

        recent_history = history[-6:]
        history_lines: list[str] = []
        for turn in recent_history:
            role = turn.get("role", "user")
            content = (turn.get("content", "") or "").strip()
            if content:
                history_lines.append(f"{role}: {content}")

        if not history_lines:
            return question

        conversation_context = "\n".join(history_lines)
        return (
            "Use the following session conversation context to resolve references like "
            "'this', 'that', 'it', and follow-up questions.\n"
            "Always answer in English only.\n"
            f"Conversation:\n{conversation_context}\n\n"
            f"Current question: {question}"
        )

    @staticmethod
    def _extract_snippets(retrieved_nodes: list, max_snippets: int = 3, max_chars: int = 900) -> list[str]:
        snippets: list[str] = []
        for retrieved in retrieved_nodes[:max_snippets]:
            source_node = getattr(retrieved, "node", retrieved)
            source_path = ""
            if hasattr(source_node, "metadata") and isinstance(source_node.metadata, dict):
                source_path = str(source_node.metadata.get("source_path", "")).strip()
            if hasattr(source_node, "get_content"):
                snippet = source_node.get_content().strip()
            else:
                snippet = str(source_node).strip()
            if snippet:
                normalized = snippet.replace("\n", " ")[:max_chars]
                if source_path:
                    snippets.append(f"Source: {source_path}\n{normalized}")
                else:
                    snippets.append(normalized)
        return snippets

    @staticmethod
    def _build_llama_messages(
        question: str,
        history: list[dict[str, str]] | None,
        context_text: str,
    ) -> list[dict[str, str]]:
        messages: list[dict[str, str]] = [
            {
                "role": "system",
                "content": (
                    "You are an assistant for HTML RAG answers. "
                    "Answer in English only. Use only the provided context. "
                    "Do not claim dataset-wide absence from limited context; "
                    "instead say you could not find evidence in retrieved context."
                ),
            }
        ]

        for turn in (history or [])[-6:]:
            role = (turn.get("role") or "").strip().lower()
            content = (turn.get("content") or "").strip()
            if role in {"user", "assistant"} and content:
                messages.append({"role": role, "content": content})

        messages.append(
            {
                "role": "user",
                "content": (
                    f"Context:\n{context_text}\n\n"
                    f"Question: {question}\n\n"
                    "Return a concise, factual answer based on context and cite log ids when available."
                ),
            }
        )
        return messages

    @staticmethod
    def _extract_threshold_from_question(question: str) -> tuple[str, float] | None:
        question_lower = question.lower()
        if not any(metric in question_lower for metric in ("recall", "precision", "accuracy")):
            return None

        symbol_match = re.search(r"(>=|<=|>|<)\s*([0-9]*\.?[0-9]+)", question_lower)
        if symbol_match:
            return symbol_match.group(1), float(symbol_match.group(2))

        phrase_patterns: list[tuple[str, str]] = [
            (r"(?:greater than|more than|above)\s*([0-9]*\.?[0-9]+)", ">"),
            (r"(?:at least|minimum of|no less than)\s*([0-9]*\.?[0-9]+)", ">="),
            (r"(?:less than|below)\s*([0-9]*\.?[0-9]+)", "<"),
            (r"(?:at most|maximum of|no more than)\s*([0-9]*\.?[0-9]+)", "<="),
        ]
        for pattern, operator in phrase_patterns:
            match = re.search(pattern, question_lower)
            if match:
                return operator, float(match.group(1))

        first_float = re.search(r"([0-9]*\.?[0-9]+)", question_lower)
        if first_float:
            return ">", float(first_float.group(1))
        return None

    @staticmethod
    def _is_greeting_or_smalltalk(question: str) -> bool:
        normalized = question.strip().lower()
        simple = {
            "hi",
            "hello",
            "hey",
            "hola",
            "yo",
            "good morning",
            "good afternoon",
            "good evening",
            "thanks",
            "thank you",
        }
        if normalized in simple:
            return True
        if len(normalized.split()) <= 3 and normalized.startswith(("hi", "hello", "hey")):
            return True
        return False

    @staticmethod
    def _normalize_metric_names(question: str) -> set[str]:
        lower = question.lower()
        metrics: set[str] = set()
        if "recall" in lower:
            metrics.add("recall")
        if "precision" in lower or "precsion" in lower:
            metrics.add("precision")
        if "accuracy" in lower:
            metrics.add("accuracy")
        return metrics

    @staticmethod
    def _compare(operator: str, value: float, threshold: float) -> bool:
        if operator == ">":
            return value > threshold
        if operator == ">=":
            return value >= threshold
        if operator == "<":
            return value < threshold
        if operator == "<=":
            return value <= threshold
        return False

    def _answer_metric_filter_query(self, question: str) -> str | None:
        threshold_data = self._extract_threshold_from_question(question)
        if threshold_data is None:
            return None
        requested_metrics = self._normalize_metric_names(question)
        if not requested_metrics:
            requested_metrics = {"recall"}

        operator, threshold = threshold_data
        if self._index is None:
            return None

        docstore = getattr(self._index, "docstore", None)
        docs = getattr(docstore, "docs", None)
        if not isinstance(docs, dict) or not docs:
            return "No ingested narrative documents are available to evaluate recall thresholds."

        entries: list[tuple[str, float, float | None, float | None]] = []
        seen_labels: set[str] = set()
        for node in docs.values():
            text = getattr(node, "text", "") or ""
            recall_match = re.search(r"mean_recall\s*=\s*([0-9]*\.?[0-9]+)", text, flags=re.IGNORECASE)
            precision_match = re.search(r"mean_precision\s*=\s*([0-9]*\.?[0-9]+)", text, flags=re.IGNORECASE)
            recall_value = float(recall_match.group(1)) if recall_match else None
            precision_value = float(precision_match.group(1)) if precision_match else None

            if "recall" in requested_metrics and recall_value is None:
                continue
            if "precision" in requested_metrics and precision_value is None:
                continue
            if "accuracy" in requested_metrics and recall_value is None:
                continue

            if "recall" in requested_metrics and not self._compare(operator, recall_value, threshold):
                continue
            if "precision" in requested_metrics and not self._compare(operator, precision_value, threshold):
                continue
            if "accuracy" in requested_metrics and not self._compare(operator, recall_value, threshold):
                continue

            metadata = getattr(node, "metadata", {})
            source_path = ""
            if isinstance(metadata, dict):
                source_path = str(metadata.get("source_path", "")).strip()

            label = Path(source_path).stem if source_path else ""
            if not label:
                log_match = re.search(r"Log:\s*([^\n\r]+?)(?:\s+Overall implication:|$)", text)
                label = log_match.group(1).strip() if log_match else "unknown_log"

            if label in seen_labels:
                continue
            seen_labels.add(label)
            entries.append((label, recall_value or 0.0, precision_value, recall_value))

        if not entries:
            metric_text = ", ".join(sorted(requested_metrics))
            return (
                f"No logs in the ingested narratives have {metric_text} {operator} {threshold:.4f}. "
                "If you expected matches, re-run ingestion with --reset-index and confirm matched_files > 0."
            )

        entries.sort(key=lambda item: item[1], reverse=True)
        metric_text = " and ".join(sorted(requested_metrics))
        lines = [f"Logs with {metric_text} {operator} {threshold:.4f}:", ""]
        for label, sort_score, precision_value, recall_value in entries[:120]:
            metrics_parts: list[str] = []
            if recall_value is not None:
                metrics_parts.append(f"recall={recall_value:.4f}")
            if precision_value is not None:
                metrics_parts.append(f"precision={precision_value:.4f}")
            if "accuracy" in requested_metrics and recall_value is not None:
                metrics_parts.append(f"accuracy≈{recall_value:.4f}")
            metrics_str = ", ".join(metrics_parts) if metrics_parts else f"score={sort_score:.4f}"
            lines.append(f"- {label}: {metrics_str}")
        if len(entries) > 120:
            lines.append("")
            lines.append(f"Showing first 120 of {len(entries)} matches.")
        return "\n".join(lines)

    def _llama_server_chat_completion(
        self,
        messages: list[dict[str, str]],
        max_tokens: int,
    ) -> tuple[str, str]:
        payload = {
            "model": Path(self._qwen_gguf_path).name,
            "messages": messages,
            "temperature": 0.1,
            "max_tokens": max_tokens,
        }
        body = json.dumps(payload).encode("utf-8")
        req = urllib_request.Request(
            url=f"{self._llama_server_base_url}/v1/chat/completions",
            data=body,
            headers={"Content-Type": "application/json"},
            method="POST",
        )
        try:
            with urllib_request.urlopen(req, timeout=120) as response:
                raw = response.read().decode("utf-8")
        except urllib_error.HTTPError as exc:
            detail = exc.read().decode("utf-8", errors="ignore")
            raise RuntimeError(f"llama-server HTTP {exc.code}: {detail}") from exc
        except urllib_error.URLError as exc:
            raise RuntimeError(f"Failed to connect to llama-server: {exc}") from exc

        data = json.loads(raw)
        choices = data.get("choices") or []
        if not choices:
            raise RuntimeError("llama-server returned no choices")
        first_choice = choices[0] or {}
        message = first_choice.get("message") or {}
        finish_reason = str(first_choice.get("finish_reason", "") or "")
        content = (message.get("content") or "").strip()
        if not content:
            raise RuntimeError("llama-server returned empty answer")
        return content, finish_reason

    def _call_llama_server(self, messages: list[dict[str, str]]) -> str:
        content, finish_reason = self._llama_server_chat_completion(
            messages=messages,
            max_tokens=self._llm_max_new_tokens,
        )
        merged = content
        round_count = 0
        while finish_reason == "length" and round_count < 3:
            continuation_messages = [
                *messages,
                {"role": "assistant", "content": merged},
                {
                    "role": "user",
                    "content": "Continue from exactly where you stopped. Do not repeat previous text.",
                },
            ]
            continuation_content, finish_reason = self._llama_server_chat_completion(
                messages=continuation_messages,
                max_tokens=max(160, self._llm_max_new_tokens // 2),
            )
            merged = f"{merged}\n{continuation_content}".strip()
            round_count += 1

        if finish_reason == "length":
            merged += "\n\n[Response truncated after multiple continuations. Increase LLM_MAX_NEW_TOKENS or narrow your query.]"
        return merged

    def _is_llama_server_healthy(self) -> bool:
        req = urllib_request.Request(f"{self._llama_server_base_url}/health", method="GET")
        try:
            with urllib_request.urlopen(req, timeout=2) as response:
                return response.status == 200
        except Exception:
            return False

    def _stop_llama_server_process(self) -> None:
        if self._llama_server_process is None:
            return
        try:
            if self._llama_server_process.poll() is None:
                self._llama_server_process.terminate()
                self._llama_server_process.wait(timeout=8)
        except Exception:
            try:
                self._llama_server_process.kill()
            except Exception:
                pass
        self._llama_server_process = None

    def _retry_llama_server_cpu(self) -> None:
        self._stop_llama_server_process()
        self._llm_n_gpu_layers = 0
        self._ensure_llama_server_runtime()

    def _ensure_llama_server_runtime(self) -> None:
        if self._is_llama_server_healthy():
            return

        if not self._llama_server_autostart:
            raise RuntimeError(
                "llama-server is not reachable and LLAMA_SERVER_AUTOSTART is disabled. "
                f"Expected endpoint: {self._llama_server_base_url}"
            )

        if not self._llama_server_path.exists():
            raise RuntimeError(
                "llama-server executable not found. "
                f"Expected: {self._llama_server_path}"
            )

        model_path = Path(self._qwen_gguf_path)
        if not model_path.exists():
            raise RuntimeError(f"Qwen GGUF model file not found: {model_path}")

        if self._llama_server_process and self._llama_server_process.poll() is None:
            return

        base_args = [
            str(self._llama_server_path),
            "-m",
            str(model_path),
            "--host",
            self._llama_server_host,
            "--port",
            str(self._llama_server_port),
            "-c",
            str(self._llm_context_window),
            "--threads",
            str(self._llm_n_threads),
            "--log-disable",
        ]
        gpu_attempts = [self._llm_n_gpu_layers]
        if self._llm_n_gpu_layers != 0:
            gpu_attempts.append(0)

        startup_errors: list[str] = []
        for gpu_layers in gpu_attempts:
            args = [*base_args, "-ngl", str(gpu_layers)]
            self._llama_server_process = subprocess.Popen(
                args,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )

            deadline = time.time() + 120
            while time.time() < deadline:
                if self._is_llama_server_healthy():
                    self._llm_n_gpu_layers = gpu_layers
                    return
                if self._llama_server_process.poll() is not None:
                    startup_errors.append(f"startup failed with n_gpu_layers={gpu_layers}")
                    break
                time.sleep(0.5)

            if self._is_llama_server_healthy():
                self._llm_n_gpu_layers = gpu_layers
                return

        raise RuntimeError(
            "Timed out waiting for llama-server to become healthy. "
            + " | ".join(startup_errors)
        )

    @staticmethod
    def _patch_llama_cpp_destructor() -> None:
        try:
            from llama_cpp import _internals as llama_internals
        except Exception:
            return

        if getattr(llama_internals.LlamaModel, "_safe_destructor_patched", False):
            return

        original_destructor = llama_internals.LlamaModel.__del__

        def _safe_destructor(self):  # type: ignore[no-untyped-def]
            try:
                if not hasattr(self, "sampler"):
                    return
                original_destructor(self)
            except Exception:
                return

        llama_internals.LlamaModel.__del__ = _safe_destructor
        llama_internals.LlamaModel._safe_destructor_patched = True

    def _rebuild_collection(self) -> None:
        if self._vector_backend == "chroma":
            try:
                self._db.delete_collection(self._collection_name)
            except Exception:
                pass

            self._collection = self._db.get_or_create_collection(self._collection_name)
            from llama_index.vector_stores.chroma import ChromaVectorStore

            self._vector_store = ChromaVectorStore(chroma_collection=self._collection)
        else:
            self._vector_store = SimpleVectorStore()
            self._storage_context = StorageContext.from_defaults(vector_store=self._vector_store)
            self._index = VectorStoreIndex(
                nodes=[],
                storage_context=self._storage_context,
            )
            self._persist_vector_store()
            return
        self._index = VectorStoreIndex.from_vector_store(self._vector_store)

    def reset_vector_store(self) -> None:
        if self._vector_backend == "chroma":
            try:
                self._db.delete_collection(self._collection_name)
            except Exception:
                pass
            self._collection = self._db.get_or_create_collection(self._collection_name)
            from llama_index.vector_stores.chroma import ChromaVectorStore

            self._vector_store = ChromaVectorStore(chroma_collection=self._collection)
            self._index = VectorStoreIndex.from_vector_store(self._vector_store)
            return

        persist_dir = self._vector_store_json_path.parent
        for filename in (
            "docstore.json",
            "index_store.json",
            "graph_store.json",
            "vector_store.json",
            "default__vector_store.json",
            "image__vector_store.json",
        ):
            target = persist_dir / filename
            if target.exists():
                target.unlink()

        self._vector_store = SimpleVectorStore()
        self._storage_context = StorageContext.from_defaults(vector_store=self._vector_store)
        self._index = None
        self._embedding_runtime_initialized = False

    def _ensure_embedding_runtime(self) -> None:
        if self._embedding_runtime_initialized:
            return

        logging.getLogger("transformers").setLevel(logging.ERROR)
        logging.getLogger("transformers.modeling_utils").setLevel(logging.ERROR)
        logging.getLogger("sentence_transformers").setLevel(logging.ERROR)
        logging.getLogger("llama_index").setLevel(logging.ERROR)
        logging.getLogger("llama_index.core").setLevel(logging.ERROR)
        logging.getLogger("llama_index.core.storage.kvstore.simple_kvstore").setLevel(logging.ERROR)

        embed_path = Path(self._embedding_model_path)
        if not embed_path.exists() or not embed_path.is_dir():
            raise RuntimeError(
                f"Local embedding model path not found: {embed_path}. "
                "Download the configured local embedding model into this folder first."
            )

        sink = io.StringIO()
        with redirect_stdout(sink), redirect_stderr(sink):
            Settings.embed_model = HuggingFaceEmbedding(
                model_name=str(embed_path),
                trust_remote_code=True,
                model_kwargs={"local_files_only": True},
                embed_batch_size=64,
            )

        if self._index is None:
            if self._vector_backend == "simple":
                try:
                    self._storage_context = StorageContext.from_defaults(
                        persist_dir=str(self._vector_store_json_path.parent)
                    )
                    self._index = load_index_from_storage(self._storage_context)
                except Exception:
                    self._vector_store = SimpleVectorStore()
                    self._storage_context = StorageContext.from_defaults(
                        vector_store=self._vector_store
                    )
                    self._index = VectorStoreIndex(
                        nodes=[],
                        storage_context=self._storage_context,
                    )
            else:
                self._index = VectorStoreIndex.from_vector_store(self._vector_store)
        self._embedding_runtime_initialized = True

    def _ensure_llm_runtime(self) -> None:
        if self._llm_runtime_initialized:
            return

        os.environ.setdefault("LLAMA_LOG_LEVEL", "0")
        self._patch_llama_cpp_destructor()

        try:
            from llama_index.llms.llama_cpp import LlamaCPP
        except ImportError as exc:
            raise RuntimeError(
                "Local GGUF model found but llama-cpp integration is missing. "
                "Install with: pip install llama-index-llms-llama-cpp llama-cpp-python"
            ) from exc

        candidate_paths: list[Path] = []
        for candidate in (Path(self._qwen_gguf_path), Path(self._qwen_fallback_gguf_path)):
            if candidate not in candidate_paths:
                candidate_paths.append(candidate)
        load_errors: list[str] = []

        for gguf_path in candidate_paths:
            if not (gguf_path.exists() and gguf_path.is_file() and gguf_path.suffix.lower() == ".gguf"):
                continue

            try:
                sink = io.StringIO()
                with redirect_stdout(sink), redirect_stderr(sink):
                    Settings.llm = LlamaCPP(
                        model_path=str(gguf_path),
                        context_window=self._llm_context_window,
                        max_new_tokens=self._llm_max_new_tokens,
                        temperature=0.1,
                        model_kwargs={
                            "n_ctx": self._llm_context_window,
                            "n_gpu_layers": self._llm_n_gpu_layers,
                            "n_threads": self._llm_n_threads,
                            "n_batch": self._llm_n_batch,
                        },
                        generate_kwargs={
                            "temperature": 0.1,
                            "top_p": 1.0,
                        },
                        verbose=False,
                    )
                self._llm_runtime_initialized = True
                return
            except Exception as exc:
                error_text = str(exc)
                if "unknown model architecture: 'qwen35'" in error_text:
                    error_text = (
                        "Model architecture 'qwen35' is not supported by current llama.cpp runtime "
                        "in this Python environment."
                    )
                load_errors.append(f"{gguf_path}: {error_text}")

        error_details = " | ".join(load_errors) if load_errors else "no valid GGUF file found"
        raise RuntimeError(
            "Failed to load local GGUF model. "
            "Set QWEN_GGUF_PATH/QWEN_FALLBACK_GGUF_PATH to a compatible local .gguf file or use a runtime that supports qwen35. "
            f"Details: {error_details}"
        )

    def add_text(
        self,
        text: str,
        source_path: str,
        duplicate_round: int,
    ) -> int:
        self._ensure_embedding_runtime()
        document = Document(
            text=text,
            metadata={
                "source_path": source_path,
                "duplicate_round": duplicate_round,
            },
        )
        nodes = self._splitter.get_nodes_from_documents([document])
        try:
            self._index.insert_nodes(nodes)
            self._persist_vector_store()
        except Exception as exc:
            message = str(exc)
            if "embedding with dimension" not in message:
                raise
            self._rebuild_collection()
            self._index.insert_nodes(nodes)
            self._persist_vector_store()
        return len(nodes)

    def answer(self, question: str, history: list[dict[str, str]] | None = None) -> str:
        if self._is_greeting_or_smalltalk(question):
            return "Hi. Ask me about ingested logs (for example: logs with recall > 0.9, precision > 0.9, or summarize a specific log id)."

        self._ensure_embedding_runtime()
        if self._index is None:
            if self._vector_backend == "simple":
                self._storage_context = StorageContext.from_defaults(
                    persist_dir=str(self._vector_store_json_path.parent)
                )
                self._index = load_index_from_storage(self._storage_context)
            else:
                self._index = VectorStoreIndex.from_vector_store(self._vector_store)

        metric_filter_answer = self._answer_metric_filter_query(question)
        if metric_filter_answer is not None:
            return metric_filter_answer

        final_query = self._build_conversational_query(question, history)

        retriever = self._index.as_retriever(similarity_top_k=self._llm_similarity_top_k)
        retrieved_nodes = retriever.retrieve(final_query)
        max_snippets = min(max(4, self._llm_similarity_top_k), 12)
        snippets = self._extract_snippets(retrieved_nodes, max_snippets=max_snippets, max_chars=720)
        context_text = "\n\n".join(snippets)[:4200]

        if self._llm_backend == "llama_server":
            try:
                self._ensure_llama_server_runtime()
                messages = self._build_llama_messages(
                    question=question,
                    history=history,
                    context_text=context_text,
                )
                try:
                    return self._call_llama_server(messages)
                except Exception as first_exc:
                    first_error = str(first_exc)
                    retryable_errors = (
                        "forcibly closed by the remote host",
                        "Failed to connect to llama-server",
                        "Connection reset",
                    )
                    if any(token in first_error for token in retryable_errors):
                        self._retry_llama_server_cpu()
                        return self._call_llama_server(messages)
                    if "exceeds the available context size" not in first_error:
                        raise
                    reduced_context = "\n\n".join(snippets[:2])[:1500]
                    reduced_messages = self._build_llama_messages(
                        question=question,
                        history=history,
                        context_text=reduced_context,
                    )
                    return self._call_llama_server(reduced_messages)
            except Exception as exc:
                runtime_error = str(exc)
                if snippets:
                    joined_snippets = "\n\n".join(snippets[:2])
                    return (
                        "LLM runtime is unavailable for generation, returning best local retrieval results.\n\n"
                        f"Runtime detail: {runtime_error}\n\n"
                        f"Top matched context:\n{joined_snippets}"
                    )
                raise RuntimeError(
                    "LLM runtime is unavailable and no relevant local context was found. "
                    f"Details: {runtime_error}"
                )

        try:
            self._ensure_llm_runtime()
        except Exception as exc:
            if snippets:
                joined_snippets = "\n\n".join(snippets)
                return (
                    "LLM runtime is unavailable in this environment for the configured GGUF model. "
                    "Returning best local retrieval results instead.\n\n"
                    f"Runtime detail: {exc}\n\n"
                    f"Top matched context:\n{joined_snippets}"
                )

            raise RuntimeError(
                "LLM runtime is unavailable and no relevant local context was found. "
                f"Details: {exc}"
            )

        query_engine = self._index.as_query_engine(similarity_top_k=self._llm_similarity_top_k)
        result = query_engine.query(final_query)
        return str(result)

    def add_many(self, docs: Iterable[tuple[str, str, int]]) -> int:
        total_chunks = 0
        for text, source_path, duplicate_round in docs:
            total_chunks += self.add_text(text, source_path, duplicate_round)
        return total_chunks
