# Work - LangGraph Local Agentic RAG

A small local **agentic AI** project using:
- **LangGraph** for agent flow orchestration
- **LangChain** runtime with local llama.cpp model
- **LangSmith** tracing (optional)
- **Langfuse** tracing callbacks (optional)
- **ChromaDB** for local RAG retrieval
- Local GGUF model: `gpt-oss-20b-F16.gguf`

## Files
- `work.py` - main app
- `work_agent/agent_graph.py` - LangGraph workflow class
- `work_agent/model_runtime.py` - llama.cpp runtime class
- `work_agent/knowledge_base.py` - Chroma knowledge base class
- `work_agent/observability.py` - LangSmith/Langfuse setup class
- `docs/knowledge.txt` - sample knowledge base
- `requirements.txt` - dependencies

## Install
```powershell
C:/Users/ouymc2/AppData/Local/Programs/Python/Python313/python.exe -m pip install -r requirements.txt
```

## Flask chat UI
Start a local web chat app:
```powershell
C:/Users/ouymc2/AppData/Local/Programs/Python/Python313/python.exe flask_app.py --model gpt-oss-20b-F16.gguf --n-ctx 512 --threads 4 --n-gpu-layers 0 --max-tokens 64
```

Then open:
- `http://127.0.0.1:5000`
- Health endpoint: `http://127.0.0.1:5000/api/health`

## Optional observability env vars
```powershell
$env:LANGSMITH_API_KEY="<key>"
$env:LANGSMITH_PROJECT="work-agent"

$env:LANGFUSE_PUBLIC_KEY="<key>"
$env:LANGFUSE_SECRET_KEY="<key>"
$env:LANGFUSE_HOST="https://cloud.langfuse.com"
```

## Quick verify (recommended first)
Indexes docs, loads model, and tests retrieval without generation:
```powershell
C:/Users/ouymc2/AppData/Local/Programs/Python/Python313/python.exe work.py --model gpt-oss-20b-F16.gguf --n-ctx 256 --threads 4 --verify-only --query "What does Work use for RAG?"
```

## Full run (agentic retrieval + answer generation)
```powershell
C:/Users/ouymc2/AppData/Local/Programs/Python/Python313/python.exe work.py --model gpt-oss-20b-F16.gguf --n-ctx 512 --threads 4 --max-tokens 64 --query "Explain this project in 3 lines"
```

## RAM-friendly flags (for large 20B model)
- `--n-gpu-layers 0` keeps layers on CPU/system RAM.
- `--no-mmap` disables mmap (default is mmap enabled).
- `--mlock` locks pages in RAM (can increase memory pressure; usually keep it off).

Example:
```powershell
C:/Users/ouymc2/AppData/Local/Programs/Python/Python313/python.exe work.py --model gpt-oss-20b-F16.gguf --n-ctx 512 --threads 4 --n-gpu-layers 0 --max-tokens 48 --query "What is Work?"
```

## Visualize project flow (.dot)
The runtime graph is in `workflow.dot`.

If Graphviz is installed, render it:
```powershell
dot -Tpng workflow.dot -o workflow.png
```

## Add your own knowledge
Put `.txt` files in `docs/` and run with `--rebuild` once:
```powershell
C:/Users/ouymc2/AppData/Local/Programs/Python/Python313/python.exe work.py --model gpt-oss-20b-F16.gguf --rebuild --verify-only
```

## Notes
- `gpt-oss-20b-F16.gguf` can be slow/heavy on CPU.
- If generation is slow, lower `--max-tokens` and `--n-ctx`, or use a smaller GGUF model.
