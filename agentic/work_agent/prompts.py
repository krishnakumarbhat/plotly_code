def planner_prompt(user_query: str) -> str:
    return (
        "You are an agent planner. Decide if retrieval is needed for the user question. "
        "Reply with one word only: RETRIEVE or DIRECT."
        f"\n\nQuestion: {user_query}"
    )


def answer_prompt(user_query: str, context: str) -> str:
    context_text = context if context else "[No retrieved context]"
    return (
        "You are Work, a local agentic AI assistant. "
        "Return only the final user-facing answer. "
        "Do not output analysis, hidden reasoning, or meta text like 'We need to answer' or 'The user says'. "
        "If context is missing, say that briefly and still help. "
        "Keep answer concise and practical."
        f"\n\nContext:\n{context_text}"
        f"\n\nUser question:\n{user_query}"
        "\n\nFinal answer:"
    )
