from __future__ import annotations

from datetime import datetime, UTC
from typing import Any


class RunStateStorage:
    def __init__(self) -> None:
        self._events: list[dict[str, Any]] = []
        self._project_results: list[dict[str, Any]] = []

    def add_event(self, stage: str, message: str, payload: dict[str, Any] | None = None) -> None:
        self._events.append(
            {
                "ts": datetime.now(UTC).isoformat(timespec="seconds"),
                "stage": stage,
                "message": message,
                "payload": payload or {},
            }
        )

    def add_project_result(self, result: dict[str, Any]) -> None:
        self._project_results.append(result)

    def get_events(self) -> list[dict[str, Any]]:
        return list(self._events)

    def get_project_results(self) -> list[dict[str, Any]]:
        return list(self._project_results)
