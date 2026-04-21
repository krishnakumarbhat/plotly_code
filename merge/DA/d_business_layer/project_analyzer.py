from __future__ import annotations

from pathlib import Path
from typing import Any

from b_persistence_layer.hdf_snapshot_repository import HdfSnapshotRepository
from b_persistence_layer.html_report_repository import HtmlReportRepository
from c_data_storage.run_state_storage import RunStateStorage


class ProjectAnalyzer:
    def __init__(
        self,
        snapshot_repository: HdfSnapshotRepository,
        html_repository: HtmlReportRepository,
        run_state: RunStateStorage,
    ) -> None:
        self._snapshot_repository = snapshot_repository
        self._html_repository = html_repository
        self._run_state = run_state

    def execute_project(self, project_name: str, config: dict[str, Any]) -> dict[str, Any]:
        project_output_dir = Path(config["output_dir"]) / project_name
        pair_list: list[dict[str, str]] = config.get("pairs", [])

        report_paths: list[Path] = []
        pair_results: list[dict[str, Any]] = []
        return_code = 0

        for index, pair in enumerate(pair_list):
            input_path = pair["input"]
            output_path = pair["output"]
            self._run_state.add_event(
                stage="pair_start",
                message=f"{project_name} pair {index + 1} started",
                payload={"input": input_path, "output": output_path},
            )

            input_snapshot = self._snapshot_repository.create_snapshot(input_path)
            output_snapshot = self._snapshot_repository.create_snapshot(output_path)

            if not input_snapshot.get("exists") or not output_snapshot.get("exists"):
                return_code = 1
                pair_result = {
                    "pair_index": index + 1,
                    "status": "failed",
                    "reason": "missing_input_or_output_file",
                    "input_exists": bool(input_snapshot.get("exists")),
                    "output_exists": bool(output_snapshot.get("exists")),
                    "input": input_path,
                    "output": output_path,
                }
                pair_results.append(pair_result)
                self._run_state.add_event(
                    stage="pair_failed",
                    message=f"{project_name} pair {index + 1} failed",
                    payload=pair_result,
                )
                continue

            metrics = self._snapshot_repository.compare_snapshots(input_snapshot, output_snapshot)
            report_path = self._html_repository.write_pair_report(
                output_dir=project_output_dir,
                project_name=project_name,
                pair_index=index,
                input_file=input_path,
                output_file=output_path,
                metrics=metrics,
            )
            report_paths.append(report_path)

            pair_result = {
                "pair_index": index + 1,
                "status": "ok",
                "report": str(report_path),
                "metrics": metrics,
            }
            pair_results.append(pair_result)
            self._run_state.add_event(
                stage="pair_done",
                message=f"{project_name} pair {index + 1} completed",
                payload={"report": str(report_path)},
            )

        index_path = self._html_repository.write_index(
            output_dir=project_output_dir,
            title=f"{project_name.upper()} KPI Reports",
            report_paths=report_paths,
        )

        result = {
            "project": project_name,
            "return_code": return_code,
            "index_html": str(index_path),
            "pair_results": pair_results,
        }
        self._run_state.add_project_result(result)
        return result
