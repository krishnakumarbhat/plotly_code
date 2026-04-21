from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path
from typing import Any


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
        self._vector_store_json_path = Path(vector_store_json_path)
        self._vector_store_json_path.parent.mkdir(parents=True, exist_ok=True)
        self._top_k = max(1, llm_similarity_top_k)
        self._chunk_size = max(400, chunk_size)
        self._chunk_overlap = max(0, min(chunk_overlap, self._chunk_size // 2))
        self._documents = self._load_documents()

    @property
    def document_count(self) -> int:
        return len(self._documents)

    def _load_documents(self) -> list[dict[str, Any]]:
        if not self._vector_store_json_path.exists():
            return []
        try:
            raw_items = json.loads(self._vector_store_json_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            return []
        if not isinstance(raw_items, list):
            return []

        documents: list[dict[str, Any]] = []
        for item in raw_items:
            if not isinstance(item, dict):
                continue
            text = str(item.get("text") or "").strip()
            source_path = str(item.get("source_path") or "").strip()
            if not text or not source_path:
                continue
            documents.append(
                {
                    "id": str(item.get("id") or self._document_id(source_path, text, 1, len(documents))),
                    "source_path": source_path,
                    "duplicate_round": int(item.get("duplicate_round") or 1),
                    "text": text,
                }
            )
        return documents

    def _persist_documents(self) -> None:
        payload = [
            {
                "id": document["id"],
                "source_path": document["source_path"],
                "duplicate_round": document["duplicate_round"],
                "text": document["text"],
            }
            for document in self._documents
        ]
        self._vector_store_json_path.write_text(
            json.dumps(payload, ensure_ascii=True, indent=2),
            encoding="utf-8",
        )

    @staticmethod
    def _document_id(source_path: str, text: str, duplicate_round: int, chunk_index: int) -> str:
        digest = hashlib.sha256(
            f"{source_path}|{duplicate_round}|{chunk_index}|{text}".encode("utf-8", errors="ignore")
        ).hexdigest()
        return digest[:16]

    def reset_vector_store(self) -> None:
        self._documents = []
        if self._vector_store_json_path.exists():
            self._vector_store_json_path.unlink()

    def add_text(self, text: str, source_path: str, duplicate_round: int = 1) -> int:
        chunks = self._chunk_text(text)
        if not chunks:
            return 0

        for chunk_index, chunk in enumerate(chunks, start=1):
            self._documents.append(
                {
                    "id": self._document_id(source_path, chunk, duplicate_round, chunk_index),
                    "source_path": source_path,
                    "duplicate_round": duplicate_round,
                    "text": chunk,
                }
            )
        self._persist_documents()
        return len(chunks)

    def _chunk_text(self, text: str) -> list[str]:
        normalized = re.sub(r"\s+", " ", text).strip()
        if not normalized:
            return []
        if len(normalized) <= self._chunk_size:
            return [normalized]

        chunks: list[str] = []
        start = 0
        while start < len(normalized):
            end = min(len(normalized), start + self._chunk_size)
            if end < len(normalized):
                boundary = normalized.rfind(" ", start, end)
                if boundary > start + 200:
                    end = boundary
            chunk = normalized[start:end].strip()
            if chunk:
                chunks.append(chunk)
            if end >= len(normalized):
                break
            start = max(start + 1, end - self._chunk_overlap)
        return chunks

    @staticmethod
    def _build_conversational_query(question: str, history: list[dict[str, str]] | None) -> str:
        if not history:
            return question

        fragments: list[str] = []
        for turn in history[-4:]:
            content = (turn.get("content") or "").strip()
            if content:
                fragments.append(content)
        if not fragments:
            return question
        return " ".join(fragments + [question])

    @staticmethod
    def _tokenize(text: str) -> list[str]:
        return [token for token in re.findall(r"[A-Za-z0-9_./:-]+", text.lower()) if len(token) > 1]

    @staticmethod
    def _is_greeting_or_smalltalk(question: str) -> bool:
        normalized = question.strip().lower()
        simple = {
            "hi",
            "hello",
            "hey",
            "good morning",
            "good afternoon",
            "good evening",
            "thanks",
            "thank you",
        }
        return normalized in simple or (len(normalized.split()) <= 3 and normalized.startswith(("hi", "hello", "hey")))

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
        return None

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

        requested_metrics = self._normalize_metric_names(question) or {"recall"}
        operator, threshold = threshold_data
        entries: list[tuple[str, float | None, float | None]] = []
        seen_labels: set[str] = set()

        for document in self._documents:
            text = document["text"]
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

            label = Path(document["source_path"]).stem or "unknown_log"
            if label in seen_labels:
                continue
            seen_labels.add(label)
            entries.append((label, recall_value, precision_value))

        if not entries:
            metric_text = " and ".join(sorted(requested_metrics))
            return f"No indexed files matched {metric_text} {operator} {threshold:.4f}."

        lines = [f"Matches for {operator} {threshold:.4f}:"]
        for label, recall_value, precision_value in entries[:50]:
            parts: list[str] = []
            if recall_value is not None:
                parts.append(f"recall={recall_value:.4f}")
            if precision_value is not None:
                parts.append(f"precision={precision_value:.4f}")
            if "accuracy" in requested_metrics and recall_value is not None:
                parts.append(f"accuracy≈{recall_value:.4f}")
            lines.append(f"- {label}: {', '.join(parts)}")
        return "\n".join(lines)

    def _search(self, question: str, history: list[dict[str, str]] | None = None, limit: int | None = None) -> list[dict[str, Any]]:
        final_query = self._build_conversational_query(question, history)
        query_tokens = self._tokenize(final_query)
        if not query_tokens:
            return []

        ranked: list[tuple[int, dict[str, Any]]] = []
        for document in self._documents:
            text_lower = document["text"].lower()
            path_lower = document["source_path"].lower()
            doc_tokens = set(self._tokenize(document["text"]))
            token_hits = sum(1 for token in query_tokens if token in doc_tokens)
            path_hits = sum(1 for token in query_tokens if token in path_lower)
            phrase_bonus = 6 if question.lower() in text_lower else 0
            score = token_hits * 3 + path_hits * 4 + phrase_bonus
            if score > 0:
                ranked.append((score, document))

        ranked.sort(key=lambda item: (-item[0], item[1]["source_path"]))
        return [document for _, document in ranked[: (limit or self._top_k)]]

    @staticmethod
    def _split_sentences(text: str) -> list[str]:
        parts = re.split(r"(?<=[.!?])\s+", text)
        return [part.strip() for part in parts if part.strip()]

    def _best_snippet(self, text: str, query: str) -> str:
        query_tokens = set(self._tokenize(query))
        best_sentence = ""
        best_score = -1
        for sentence in self._split_sentences(text):
            sentence_tokens = set(self._tokenize(sentence))
            score = sum(1 for token in query_tokens if token in sentence_tokens)
            if score > best_score:
                best_sentence = sentence
                best_score = score
        if best_sentence:
            return best_sentence[:360]
        return text[:360]

    def answer(self, question: str, history: list[dict[str, str]] | None = None) -> str:
        if self._is_greeting_or_smalltalk(question):
            return "Ask about KPI inputs, runtime wiring, HTML reports, or recent files in the indexed workspace."

        metric_answer = self._answer_metric_filter_query(question)
        if metric_answer:
            return metric_answer

        if not self._documents:
            return "No files are indexed yet. Start the service with a valid HTML_ROOT_PATH or call POST /ingest first."

        matches = self._search(question, history=history)
        if not matches:
            return "I could not find relevant indexed content for that question. Try a file name, tool name, KPI mode, or a more specific phrase."

        lines = ["Top matches from the indexed workspace:"]
        seen_sources: set[str] = set()
        for document in matches[:3]:
            source_path = document["source_path"]
            if source_path in seen_sources:
                continue
            seen_sources.add(source_path)
            snippet = self._best_snippet(document["text"], question)
            lines.append(f"- {Path(source_path).name}: {snippet}")
        lines.append("")
        lines.append("Sources:")
        for source_path in list(seen_sources)[:5]:
            lines.append(f"- {source_path}")
        return "\n".join(lines)
