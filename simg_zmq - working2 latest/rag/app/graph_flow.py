from __future__ import annotations

from langgraph.graph import END, START, StateGraph

from app.rag_engine import RagEngine
from app.sqlite_store import SQLiteLogStore


class QueryGraph:
    def __init__(self, rag_engine: RagEngine, sqlite_store: SQLiteLogStore) -> None:
        self.rag_engine = rag_engine
        self.sqlite_store = sqlite_store

        graph_builder = StateGraph(dict)
        graph_builder.add_node("answer_question", self._answer_question)
        graph_builder.add_edge(START, "answer_question")
        graph_builder.add_edge("answer_question", END)
        self.graph = graph_builder.compile()

    def _answer_question(self, state: dict) -> dict:
        question = state.get("question", "")
        history = state.get("history")
        answer = self.rag_engine.answer(question, history=history)
        self.sqlite_store.log_query(question=question, answer=answer)
        return {"answer": answer}

    def run(self, question: str, history: list[dict[str, str]] | None = None) -> str:
        output = self.graph.invoke({"question": question, "history": history or []})
        return output["answer"]
