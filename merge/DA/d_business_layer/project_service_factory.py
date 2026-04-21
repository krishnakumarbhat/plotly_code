from __future__ import annotations

from d_business_layer.can_kpi_business import CanKpiBusiness
from d_business_layer.int_plot_business import IntPlotBusiness
from d_business_layer.project_analyzer import ProjectAnalyzer
from d_business_layer.udp_kpi_business import UdpKpiBusiness


class ProjectServiceFactory:
    def __init__(self, analyzer: ProjectAnalyzer) -> None:
        self._analyzer = analyzer

    def create(self, project_name: str):
        if project_name == "can":
            return CanKpiBusiness(self._analyzer)
        if project_name == "udp":
            return UdpKpiBusiness(self._analyzer)
        if project_name == "int":
            return IntPlotBusiness(self._analyzer)
        raise ValueError(f"Unsupported project name: {project_name}")
