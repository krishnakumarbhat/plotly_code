from __future__ import annotations

import argparse
from pathlib import Path

from a_config_layer.da_config_loader import DaConfigLoader
from b_persistence_layer.hdf_snapshot_repository import HdfSnapshotRepository
from b_persistence_layer.html_report_repository import HtmlReportRepository
from c_data_storage.run_state_storage import RunStateStorage
from d_business_layer.da_router_business import DaRouterBusiness
from d_business_layer.project_analyzer import ProjectAnalyzer
from d_business_layer.project_service_factory import ProjectServiceFactory
from e_presentation_layer.da_result_presenter import DaResultPresenter


class DaMainApp:
    def __init__(self) -> None:
        self._workspace_root = Path(__file__).resolve().parent

    def _parse_args(self) -> argparse.Namespace:
        parser = argparse.ArgumentParser(description="Standalone DA runner (CAN/UDP/INT)")
        parser.add_argument("--config", default="config.json", help="Path to DA config JSON")
        parser.add_argument("--out", default="out_html", help="Output folder for generated artifacts")
        parser.add_argument("--source", default=None, help="Override HDF source selection (MUDP/BORDNET)")
        parser.add_argument("--run-int", action="store_true", help="Force running INT project")
        return parser.parse_args()

    def run(self) -> int:
        args = self._parse_args()
        run_state = RunStateStorage()
        loader = DaConfigLoader(self._workspace_root)
        config = loader.load(
            config_path=args.config,
            output_dir=args.out,
            source_override=args.source,
            run_int_override=True if args.run_int else None,
        )
        run_state.add_event(stage="config_loaded", message="Configuration loaded", payload=config)

        snapshot_repository = HdfSnapshotRepository()
        html_repository = HtmlReportRepository()
        analyzer = ProjectAnalyzer(snapshot_repository, html_repository, run_state)
        factory = ProjectServiceFactory(analyzer)
        router = DaRouterBusiness(factory, run_state)

        project_results = router.execute(config)

        presenter = DaResultPresenter(html_repository, run_state)
        output_paths = presenter.write_outputs(config, project_results)
        text = presenter.build_console_text(
            config,
            project_results,
            output_paths["summary"],
            output_paths["debug"],
        )
        print(text)

        failed = [item for item in project_results if int(item.get("return_code", 1)) != 0]
        return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(DaMainApp().run())
