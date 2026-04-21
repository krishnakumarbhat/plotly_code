from pathlib import Path
from typing import List

import chromadb

from work_agent.embeddings import HashEmbeddings


class ChromaKnowledgeBase:
    def __init__(self, chroma_path: str, collection_name: str):
        self.client = chromadb.PersistentClient(path=chroma_path)
        self.collection = self.client.get_or_create_collection(name=collection_name)
        self.embeddings = HashEmbeddings(dim=384)

    def _chunk_text(self, text: str, chunk_size: int = 420, overlap: int = 80) -> List[str]:
        normalized = " ".join(text.split())
        if not normalized:
            return []

        chunks = []
        start = 0
        length = len(normalized)
        while start < length:
            end = min(start + chunk_size, length)
            chunk = normalized[start:end].strip()
            if chunk:
                chunks.append(chunk)
            if end >= length:
                break
            start = max(end - overlap, start + 1)
        return chunks

    def ingest(self, docs_path: str, rebuild: bool = False) -> int:
        root = Path(docs_path)
        if not root.exists():
            raise FileNotFoundError(f"Docs directory not found: {root}")

        if rebuild:
            existing_ids = self.collection.get(include=[]).get("ids", [])
            if existing_ids:
                self.collection.delete(ids=existing_ids)

        ids = []
        documents = []
        metadatas = []

        for file_path in root.rglob("*.txt"):
            raw = file_path.read_text(encoding="utf-8", errors="ignore")
            chunks = self._chunk_text(raw)
            for index, chunk in enumerate(chunks):
                ids.append(f"{file_path.name}-{index}")
                documents.append(chunk)
                metadatas.append({"source": str(file_path)})

        if not ids:
            return 0

        current_ids = set(self.collection.get(include=[]).get("ids", []))
        filtered = [(doc_id, doc, meta) for doc_id, doc, meta in zip(ids, documents, metadatas) if doc_id not in current_ids]
        if not filtered:
            return 0

        add_ids, add_docs, add_meta = zip(*filtered)
        add_embeddings = self.embeddings.embed_documents(list(add_docs))

        self.collection.add(
            ids=list(add_ids),
            documents=list(add_docs),
            metadatas=list(add_meta),
            embeddings=add_embeddings,
        )
        return len(add_ids)

    def retrieve(self, query: str, top_k: int = 3) -> str:
        results = self.collection.query(
            query_embeddings=[self.embeddings.embed_query(query)],
            n_results=top_k,
            include=["documents", "metadatas", "distances"],
        )

        docs = results.get("documents", [[]])[0]
        metas = results.get("metadatas", [[]])[0]
        if not docs:
            return ""

        output = []
        for idx, (doc, meta) in enumerate(zip(docs, metas), start=1):
            source = (meta or {}).get("source", "unknown")
            output.append(f"[{idx}] Source: {source}\n{doc}")
        return "\n\n".join(output)
