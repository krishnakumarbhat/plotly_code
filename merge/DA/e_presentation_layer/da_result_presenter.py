from __future__ import annotations

from pathlib import Path
from typing import Any

from b_persistence_layer.html_report_repository import HtmlReportRepository
from c_data_storage.run_state_storage import RunStateStorage


class DaResultPresenter:
    def __init__(self, html_repository: HtmlReportRepository, run_state: RunStateStorage) -> None:
        self._html_repository = html_repository
        self._run_state = run_state

    def write_outputs(self, config: dict[str, Any], project_results: list[dict[str, Any]]) -> dict[str, Path]:
        output_dir = Path(config["output_dir"])
        summary_payload = {
            "hdf_source_selection": config.get("hdf_source_selection"),
            "hdf_file_mode": config.get("hdf_file_mode"),
            "xml_config": config.get("xml_config"),
            "inputs_json": config.get("inputs_json"),
            "results": project_results,
        }
        debug_payload = {
            "events": self._run_state.get_events(),
            "project_results": self._run_state.get_project_results(),
        }

        summary_path = self._html_repository.write_json(output_dir / "da_run_summary.json", summary_payload)
        debug_path = self._html_repository.write_json(output_dir / "da_debug_trace.json", debug_payload)
        return {"summary": summary_path, "debug": debug_path}

    def build_console_text(
        self,
        config: dict[str, Any],
        project_results: list[dict[str, Any]],
        summary_path: Path,
        debug_path: Path,
    ) -> str:
        lines = [
            f"DA standalone run complete (source={config.get('hdf_source_selection')}, mode={config.get('hdf_file_mode')})",
            f"Summary JSON: {summary_path}",
            f"Debug trace JSON: {debug_path}",
        ]
        for result in project_results:
            lines.append(f"- {result.get('project')}: rc={result.get('return_code')} -> {result.get('index_html')}")
        return "\n".join(lines)
