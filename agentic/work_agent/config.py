from dataclasses import dataclass


@dataclass
class WorkConfig:
    model_path: str
    docs_path: str
    chroma_path: str
    collection_name: str
    query: str
    n_ctx: int
    threads: int
    max_tokens: int
    n_gpu_layers: int
    use_mmap: bool
    use_mlock: bool
    top_k: int
    verify_only: bool
    rebuild: bool

    @classmethod
    def from_args(cls, args):
        return cls(
            model_path=args.model,
            docs_path=args.docs,
            chroma_path=args.chroma_path,
            collection_name=args.collection,
            query=args.query,
            n_ctx=args.n_ctx,
            threads=args.threads,
            max_tokens=args.max_tokens,
            n_gpu_layers=args.n_gpu_layers,
            use_mmap=not args.no_mmap,
            use_mlock=args.mlock,
            top_k=args.top_k,
            verify_only=getattr(args, "verify_only", False),
            rebuild=getattr(args, "rebuild", False),
        )
