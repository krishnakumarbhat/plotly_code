class ResponseSanitizer:
    def __init__(self):
        self.bad_markers = [
            "we need to answer",
            "the user says",
            "<|channel|>",
            "analysis<|message|>",
        ]

    def _looks_meta(self, text: str) -> bool:
        normalized = (text or "").strip().lower()
        if not normalized:
            return True
        if any(marker in normalized for marker in self.bad_markers):
            return True

        alpha_count = sum(1 for ch in normalized if ch.isalpha())
        punctuation_count = sum(1 for ch in normalized if ch in '"\'`.,:;!?|<>[]{}()')
        total = len(normalized)
        if total > 0 and (alpha_count / total) < 0.35:
            return True
        if punctuation_count > alpha_count:
            return True
        return False

    def sanitize(self, llm, question: str, context: str, answer: str) -> str:
        raw = (answer or "").strip()
        if not self._looks_meta(raw):
            return raw

        rewrite_prompt = (
            "Rewrite the draft into a clean final reply for the user. "
            "Do not include analysis, internal thoughts, or meta commentary. "
            "Keep it direct and short."
            f"\n\nQuestion:\n{question}"
            f"\n\nContext:\n{context if context else '[No retrieved context]'}"
            f"\n\nDraft:\n{raw if raw else '[empty]'}"
            "\n\nFinal answer:"
        )
        rewritten = llm.invoke(rewrite_prompt)
        if hasattr(rewritten, "content"):
            rewritten_text = str(rewritten.content).strip()
        else:
            rewritten_text = str(rewritten).strip()
        return rewritten_text if rewritten_text else "I am Work, your local AI assistant."
