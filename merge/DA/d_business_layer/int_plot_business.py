from __future__ import annotations

from typing import Any

from d_business_layer.project_analyzer import ProjectAnalyzer


class IntPlotBusiness:
    def __init__(self, analyzer: ProjectAnalyzer) -> None:
        self._analyzer = analyzer

    def execute(self, config: dict[str, Any]) -> dict[str, Any]:
        return self._analyzer.execute_project("int", config)
