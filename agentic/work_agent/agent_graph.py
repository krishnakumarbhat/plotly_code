from typing import List, Optional, TypedDict

from langgraph.graph import END, StateGraph

from work_agent.knowledge_base import ChromaKnowledgeBase
from work_agent.prompts import answer_prompt
from work_agent.response_sanitizer import ResponseSanitizer


class AgentState(TypedDict):
    query: str
    needs_retrieval: bool
    context: str
    answer: str


class WorkAgentGraph:
    def __init__(self, llm, knowledge_base: ChromaKnowledgeBase, top_k: int = 3, callbacks: Optional[List[object]] = None):
        self.llm = llm
        self.knowledge_base = knowledge_base
        self.top_k = top_k
        self.callbacks = callbacks or []
        self.sanitizer = ResponseSanitizer()

    def _invoke_llm(self, prompt: str) -> str:
        config = {"callbacks": self.callbacks} if self.callbacks else None
        response = self.llm.invoke(prompt, config=config)
        if hasattr(response, "content"):
            return str(response.content).strip()
        return str(response).strip()

    def _plan_node(self, state: AgentState) -> AgentState:
        query = state["query"].strip().lower()
        greeting_terms = ["hi", "hello", "hey", "who are you", "what are you"]
        state["needs_retrieval"] = not any(term in query for term in greeting_terms)
        return state

    def _retrieve_node(self, state: AgentState) -> AgentState:
        state["context"] = self.knowledge_base.retrieve(state["query"], top_k=self.top_k)
        return state

    def _direct_node(self, state: AgentState) -> AgentState:
        state["context"] = ""
        return state

    def _answer_node(self, state: AgentState) -> AgentState:
        prompt = answer_prompt(state["query"], state.get("context", ""))
        raw_answer = self._invoke_llm(prompt)
        state["answer"] = self.sanitizer.sanitize(
            llm=self.llm,
            question=state["query"],
            context=state.get("context", ""),
            answer=raw_answer,
        )
        return state

    def build(self):
        graph = StateGraph(AgentState)
        graph.add_node("plan", self._plan_node)
        graph.add_node("retrieve", self._retrieve_node)
        graph.add_node("direct", self._direct_node)
        graph.add_node("answer", self._answer_node)

        graph.set_entry_point("plan")
        graph.add_conditional_edges(
            "plan",
            lambda state: "retrieve" if state.get("needs_retrieval") else "direct",
            {"retrieve": "retrieve", "direct": "direct"},
        )
        graph.add_edge("retrieve", "answer")
        graph.add_edge("direct", "answer")
        graph.add_edge("answer", END)

        return graph.compile()
