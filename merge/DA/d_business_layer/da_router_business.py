from __future__ import annotations

from typing import Any

from c_data_storage.run_state_storage import RunStateStorage
from d_business_layer.project_service_factory import ProjectServiceFactory


class DaRouterBusiness:
    def __init__(self, service_factory: ProjectServiceFactory, run_state: RunStateStorage) -> None:
        self._service_factory = service_factory
        self._run_state = run_state

    def _select_primary_project(self, hdf_source_selection: str) -> str:
        return "udp" if (hdf_source_selection or "").upper() == "MUDP" else "can"

    def execute(self, config: dict[str, Any]) -> list[dict[str, Any]]:
        selected_projects: list[str] = []
        if bool(config.get("run_int", False)):
            selected_projects.append("int")
        selected_projects.append(self._select_primary_project(config.get("hdf_source_selection", "")))

        self._run_state.add_event(
            stage="routing",
            message="Selected projects for execution",
            payload={"projects": selected_projects},
        )

        results: list[dict[str, Any]] = []
        for project_name in selected_projects:
            service = self._service_factory.create(project_name)
            result = service.execute(config)
            results.append(result)
        return results
