import os
import sys
import importlib
from pathlib import Path


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]
    os.environ.setdefault('RAG_AUTO_INGEST_ON_START', 'false')
    os.environ.setdefault('LLAMA_SERVER_PORT', '8082')
    os.environ.setdefault('LLAMA_SERVER_BASE_URL', 'http://127.0.0.1:8082')
    sys.path.insert(0, str(repo_root / 'rag'))

    Application = importlib.import_module('app.main').Application

    app = Application()
    app.rag_engine.reset_vector_store()
    app.rag_engine.add_text(
        'The local workspace guide uses the Qwen_Qwen3.5-2B-Q5_K_S.gguf model for concise answers.',
        'smoke.txt',
        1,
    )
    answer = app.rag_engine.answer('Which model is used by the local workspace guide?')
    print('ANSWER_START')
    print(answer)
    print('ANSWER_END')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())