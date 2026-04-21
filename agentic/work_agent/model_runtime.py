from langchain_community.chat_models import ChatLlamaCpp


class LocalLlamaRuntime:
    def __init__(
        self,
        model_path: str,
        n_ctx: int,
        n_threads: int,
        max_tokens: int,
        n_gpu_layers: int,
        use_mmap: bool,
        use_mlock: bool,
    ):
        self.model_path = model_path
        self.n_ctx = n_ctx
        self.n_threads = n_threads
        self.max_tokens = max_tokens
        self.n_gpu_layers = n_gpu_layers
        self.use_mmap = use_mmap
        self.use_mlock = use_mlock

    def build(self) -> ChatLlamaCpp:
        return ChatLlamaCpp(
            model_path=self.model_path,
            n_ctx=self.n_ctx,
            n_threads=self.n_threads,
            max_tokens=self.max_tokens,
            temperature=0.2,
            top_p=0.9,
            repeat_penalty=1.1,
            stop=["<|channel|>", "analysis<|message|>", "\nUser:", "\nQuestion:"],
            n_gpu_layers=self.n_gpu_layers,
            use_mmap=self.use_mmap,
            use_mlock=self.use_mlock,
            verbose=False,
        )
