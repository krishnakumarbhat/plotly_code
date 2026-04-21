import os
from typing import List

from langsmith import Client


class TelemetryManager:
    def __init__(self, project_name: str = "work-agent"):
        self.project_name = project_name

    def setup_langsmith(self) -> bool:
        has_key = bool(os.getenv("LANGSMITH_API_KEY"))
        if not has_key:
            return False

        os.environ.setdefault("LANGSMITH_TRACING", "true")
        os.environ.setdefault("LANGSMITH_PROJECT", self.project_name)
        Client()
        return True

    def build_langfuse_callbacks(self) -> List[object]:
        public_key = os.getenv("LANGFUSE_PUBLIC_KEY")
        secret_key = os.getenv("LANGFUSE_SECRET_KEY")
        host = os.getenv("LANGFUSE_HOST", "https://cloud.langfuse.com")
        if not public_key or not secret_key:
            return []

        try:
            from langfuse.langchain import CallbackHandler

            handler = CallbackHandler(
                public_key=public_key,
                secret_key=secret_key,
                host=host,
            )
            return [handler]
        except Exception:
            return []
