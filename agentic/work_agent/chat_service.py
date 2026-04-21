from pathlib import Path

from work_agent.agent_graph import WorkAgentGraph
from work_agent.config import WorkConfig
from work_agent.knowledge_base import ChromaKnowledgeBase
from work_agent.model_runtime import LocalLlamaRuntime
from work_agent.observability import TelemetryManager


class WorkChatService:
    def __init__(self, config: WorkConfig):
        self.config = config
        if not Path(config.model_path).exists():
            raise FileNotFoundError(f"Model file not found: {config.model_path}")

        self.telemetry = TelemetryManager(project_name="work-agent")
        self.langsmith_enabled = self.telemetry.setup_langsmith()
        self.callbacks = self.telemetry.build_langfuse_callbacks()

        self.kb = ChromaKnowledgeBase(config.chroma_path, config.collection_name)
        self.indexed_chunks = self.kb.ingest(config.docs_path, rebuild=config.rebuild)

        runtime = LocalLlamaRuntime(
            model_path=config.model_path,
            n_ctx=config.n_ctx,
            n_threads=config.threads,
            max_tokens=config.max_tokens,
            n_gpu_layers=config.n_gpu_layers,
            use_mmap=config.use_mmap,
            use_mlock=config.use_mlock,
        )
        self.llm = runtime.build()
        self.graph = WorkAgentGraph(
            llm=self.llm,
            knowledge_base=self.kb,
            top_k=config.top_k,
            callbacks=self.callbacks,
        ).build()
        self.last_user_query = ""
        self.last_answer = ""

    def _identity_reply(self) -> str:
        return (
            "I am Work, your local AI assistant running on your GGUF model with LangGraph + Chroma RAG. "
            "I can answer questions from your local docs and general prompts."
        )

    def _identity_detail_reply(self) -> str:
        return (
            "I am Work, a local agentic assistant built with Flask UI, LangGraph workflow, and Chroma retrieval. "
            "Your model runs locally from gpt-oss-20b-F16.gguf, and I use retrieved context from docs/ when needed. "
            "This keeps your data local while still supporting RAG-based answers."
        )

    def ask(self, query: str) -> dict:
        normalized = query.strip().lower()

        if any(key in normalized for key in ["who are you", "what are you", "hi", "hello", "hey"]):
            answer = self._identity_reply()
            self.last_user_query = query
            self.last_answer = answer
            return {
                "answer": answer,
                "context": "",
                "needs_retrieval": False,
            }

        if any(key in normalized for key in ["tell it in more detail", "more detail", "explain more"]):
            answer = self._identity_detail_reply() if self.last_answer else "Please tell me what topic you want in more detail."
            self.last_user_query = query
            self.last_answer = answer
            return {
                "answer": answer,
                "context": "",
                "needs_retrieval": False,
            }

        query_for_graph = query
        if self.last_answer and any(key in normalized for key in ["it", "that", "this", "more"]):
            query_for_graph = f"{query}\n\nPrevious assistant answer:\n{self.last_answer}"

        state = {
            "query": query_for_graph,
            "needs_retrieval": False,
            "context": "",
            "answer": "",
        }
        result = self.graph.invoke(state)
        answer = result.get("answer", "")
        self.last_user_query = query
        self.last_answer = answer
        return {
            "answer": answer,
            "context": result.get("context", ""),
            "needs_retrieval": bool(result.get("needs_retrieval", False)),
        }

    def health(self) -> dict:
        return {
            "project": "Work",
            "indexed_chunks": self.indexed_chunks,
            "langsmith_enabled": self.langsmith_enabled,
            "langfuse_enabled": bool(self.callbacks),
            "model": self.config.model_path,
        }
