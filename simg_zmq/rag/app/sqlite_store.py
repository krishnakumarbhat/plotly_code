from __future__ import annotations

import sqlite3
from pathlib import Path
from typing import Any


class SQLiteLogStore:
    def __init__(self, db_path: Path) -> None:
        self.db_path = db_path
        self._init_db()

    def _init_db(self) -> None:
        with sqlite3.connect(self.db_path) as conn:
            conn.execute(
                """
                CREATE TABLE IF NOT EXISTS ingestions (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    source_path TEXT NOT NULL,
                    text_hash TEXT NOT NULL,
                    chunk_count INTEGER NOT NULL,
                    duplicate_round INTEGER NOT NULL,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                )
                """
            )
            conn.execute(
                """
                CREATE TABLE IF NOT EXISTS queries (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    question TEXT NOT NULL,
                    answer TEXT NOT NULL,
                    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                )
                """
            )

    def ingestion_exists(self, source_path: str, text_hash: str) -> bool:
        with sqlite3.connect(self.db_path) as conn:
            row = conn.execute(
                "SELECT 1 FROM ingestions WHERE source_path = ? AND text_hash = ? LIMIT 1",
                (source_path, text_hash),
            ).fetchone()
        return row is not None

    def log_ingestion(
        self,
        source_path: str,
        text_hash: str,
        chunk_count: int,
        duplicate_round: int,
    ) -> None:
        with sqlite3.connect(self.db_path) as conn:
            conn.execute(
                """
                INSERT INTO ingestions (source_path, text_hash, chunk_count, duplicate_round)
                VALUES (?, ?, ?, ?)
                """,
                (source_path, text_hash, chunk_count, duplicate_round),
            )

    def log_query(self, question: str, answer: str) -> None:
        with sqlite3.connect(self.db_path) as conn:
            conn.execute(
                "INSERT INTO queries (question, answer) VALUES (?, ?)",
                (question, answer),
            )

    def clear_ingestions(self) -> None:
        with sqlite3.connect(self.db_path) as conn:
            conn.execute("DELETE FROM ingestions")

    def clear_queries(self) -> None:
        with sqlite3.connect(self.db_path) as conn:
            conn.execute("DELETE FROM queries")

    def stats(self) -> dict[str, Any]:
        with sqlite3.connect(self.db_path) as conn:
            ingestions = conn.execute("SELECT COUNT(*) FROM ingestions").fetchone()[0]
            queries = conn.execute("SELECT COUNT(*) FROM queries").fetchone()[0]
        return {"ingestions": ingestions, "queries": queries}
