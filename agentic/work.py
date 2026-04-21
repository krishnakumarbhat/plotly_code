import argparse
import os
from pathlib import Path

from work_agent.agent_graph import WorkAgentGraph
from work_agent.config import WorkConfig
from work_agent.knowledge_base import ChromaKnowledgeBase
from work_agent.model_runtime import LocalLlamaRuntime
from work_agent.observability import TelemetryManager

PROJECT_NAME = "Work"
DEFAULT_MODEL = "gpt-oss-20b-F16.gguf"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Work: LangGraph + LangChain + Chroma local agentic RAG")
    parser.add_argument("--model", default=DEFAULT_MODEL, help="Path to GGUF model")
    parser.add_argument("--docs", default="docs", help="Path to folder with .txt knowledge files")
    parser.add_argument("--query", default="What is this project about?", help="Question for the agent")

    parser.add_argument("--chroma-path", default="chroma_db", help="Chroma persistence path")
    parser.add_argument("--collection", default="work_knowledge", help="Chroma collection name")
    parser.add_argument("--top-k", type=int, default=3, help="Retrieved chunks count")

    parser.add_argument("--n-ctx", type=int, default=1024, help="Model context window")
    parser.add_argument("--threads", type=int, default=max((os.cpu_count() or 4) - 1, 1), help="CPU threads")
    parser.add_argument("--max-tokens", type=int, default=128, help="Max output tokens")
    parser.add_argument("--n-gpu-layers", type=int, default=0, help="GPU offload layers (0=CPU/system RAM)")
    parser.add_argument("--no-mmap", action="store_true", help="Disable memory-mapped model loading")
    parser.add_argument("--mlock", action="store_true", help="Lock model pages in RAM (can increase RAM pressure)")

    parser.add_argument("--verify-only", action="store_true", help="Index + retrieve test without generation")
    parser.add_argument("--rebuild", action="store_true", help="Rebuild index from docs")
    return parser


def main():
    args = build_parser().parse_args()
    config = WorkConfig.from_args(args)

    if not Path(config.model_path).exists():
        raise FileNotFoundError(f"Model file not found: {config.model_path}")

    telemetry = TelemetryManager(project_name="work-agent")
    langsmith_on = telemetry.setup_langsmith()
    callbacks = telemetry.build_langfuse_callbacks()

    kb = ChromaKnowledgeBase(config.chroma_path, config.collection_name)
    added = kb.ingest(config.docs_path, rebuild=config.rebuild)
    print(f"[{PROJECT_NAME}] Indexed chunks added: {added}", flush=True)

    if langsmith_on:
        print(f"[{PROJECT_NAME}] LangSmith tracing enabled", flush=True)
    if callbacks:
        print(f"[{PROJECT_NAME}] Langfuse callback enabled", flush=True)

    runtime = LocalLlamaRuntime(
        model_path=config.model_path,
        n_ctx=config.n_ctx,
        n_threads=config.threads,
        max_tokens=config.max_tokens,
        n_gpu_layers=config.n_gpu_layers,
        use_mmap=config.use_mmap,
        use_mlock=config.use_mlock,
    )
    llm = runtime.build()

    print(f"[{PROJECT_NAME}] Query: {config.query}", flush=True)

    if config.verify_only:
        context_preview = kb.retrieve(config.query, top_k=config.top_k)
        print("\n=== RETRIEVED CONTEXT PREVIEW ===")
        print(context_preview[:700] if context_preview else "[No context found]")
        print(f"\n[{PROJECT_NAME}] Verification complete (generation skipped).", flush=True)
        return

    graph = WorkAgentGraph(llm=llm, knowledge_base=kb, top_k=config.top_k, callbacks=callbacks).build()
    result = graph.invoke(
        {
            "query": config.query,
            "needs_retrieval": False,
            "context": "",
            "answer": "",
        }
    )

    print("\n=== ANSWER ===")
    print(result.get("answer", "[No answer generated]"))


if __name__ == "__main__":
    main()
