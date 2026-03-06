"""CAN Radar KPI — main entry point.

Usage
-----
    python kpi_main.py [kpi.json] [output_dir]

- If kpi.json is omitted, resolves like before (local / bundled / dev).
- If output_dir is omitted, writes to current directory.
- If kpi.json contains multiple INPUT_HDF/OUTPUT_HDF entries, writes one HTML
    per pair and an index.html linking to them.
"""

import logging
import sys
from pathlib import Path
from typing import Optional, List, Tuple

from c_business_layer.kpi_business import KpiBusiness
from a_persistence_layer.json_parser import KpiJsonParser
from d_presentation_layer.kpi_html_gen import KpiHtmlGen
import numpy as np

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s  %(levelname)-8s  %(name)s  %(message)s",
)
logger = logging.getLogger(__name__)


class KpiMain:
    def __init__(self, business: Optional[KpiBusiness] = None):
        self._json = KpiJsonParser()
        self._business = business or KpiBusiness()
        self._html = KpiHtmlGen()

    def run(
        self, config_path: str = "kpi.json", output_dir: str | Path = "out_html"
    ) -> Path:
        config_path = str(config_path)
        out_dir = Path(output_dir).expanduser().resolve()
        out_dir.mkdir(parents=True, exist_ok=True)

        logger.info(f"Config: {config_path}")
        logger.info(f"Output dir: {out_dir}")

        cfg = self._json.parse(config_path)
        input_paths: List[str] = cfg.get("INPUT_HDF", [])
        output_paths: List[str] = cfg.get("OUTPUT_HDF", [])
        n_logs = max(len(input_paths), len(output_paths))

        if n_logs <= 0:
            raise ValueError("No INPUT_HDF/OUTPUT_HDF entries found")

        report_files: List[Tuple[str, str]] = []  # (display_name, filename)
        failures: List[str] = []
        for i in range(n_logs):
            in_path = input_paths[i] if i < len(input_paths) else None
            out_path = output_paths[i] if i < len(output_paths) else None

            try:
                stem = (
                    Path(in_path).stem
                    if in_path
                    else (Path(out_path).stem if out_path else f"log_{i + 1}")
                )
                safe = "".join(
                    ch if ch.isalnum() or ch in ("-", "_", ".") else "_" for ch in stem
                )
                filename = f"{safe}.html"
                used = {f for _, f in report_files}
                if filename in used:
                    filename = f"{safe}_{i + 1}.html"

                title = f"PCAN KPI — {stem}"
                result = self.run_pair(in_path, out_path, title=title)
                report_path = out_dir / filename
                report_path.write_text(result["html"], encoding="utf-8")
                logger.info(f"Wrote report to {report_path}")
                report_files.append((stem, filename))
            except Exception as exc:
                pair_name = f"input={in_path}, output={out_path}"
                logger.exception(f"Failed report generation for {pair_name}: {exc}")
                failures.append(pair_name)

        if not report_files:
            raise RuntimeError("No report generated. Check logs for failed HDF pairs.")

        index_path = out_dir / "index.html"
        index_path.write_text(self._build_index(report_files), encoding="utf-8")
        logger.info(f"Wrote index to {index_path}")
        if failures:
            logger.warning(f"Failed pairs: {len(failures)}")
        return index_path

    def run_pair(
        self,
        input_hdf: Optional[str],
        output_hdf: Optional[str],
        title: str = "PCAN Detection KPI",
    ) -> dict:
        if not input_hdf and not output_hdf:
            raise ValueError("At least one of input_hdf/output_hdf must be provided")

        in_parsed = self._business._hdf.parse_file(input_hdf) if input_hdf else {}
        out_parsed = self._business._hdf.parse_file(output_hdf) if output_hdf else {}
        in_stores = self._business._hdf.extract_storages(in_parsed)
        out_stores = self._business._hdf.extract_storages(out_parsed)

        all_sensors = sorted(
            set(in_stores.keys()) | set(out_stores.keys()),
            key=lambda s: self._business.SENSOR_ORDER.index(s)
            if s in self._business.SENSOR_ORDER
            else 99,
        )

        match_results = {
            sensor_id: self._business._compute_match_pct(
                in_stores.get(sensor_id), out_stores.get(sensor_id), sensor_id
            )
            for sensor_id in all_sensors
        }

        summary_headers, summary_rows = self._business._build_summary_tables(
            in_stores, out_stores, all_sensors
        )
        summary_html = self._html.stats_table(
            "Overview — All Sensors", summary_headers, summary_rows
        )

        sensor_tabs = {}
        for sensor_id in all_sensors:
            label = self._business.FRIENDLY.get(sensor_id, sensor_id)
            result = match_results.get(sensor_id, {})
            sensor_tabs[label] = self._html.build_sensor_tab(
                label,
                result.get("scan", np.array([], dtype=np.int64)),
                {
                    "Overall": result.get("overall", np.array([], dtype=np.float16)),
                    "Precision": result.get(
                        "precision", np.array([], dtype=np.float16)
                    ),
                    "Recall": result.get("recall", np.array([], dtype=np.float16)),
                    "F1": result.get("f1", np.array([], dtype=np.float16)),
                    "Accuracy": result.get("accuracy", np.array([], dtype=np.float16)),
                },
                result.get("per_signal", {}),
            )

        kpi_rows: List[List[str]] = []
        radar_plot_data = {}
        for sensor_id in all_sensors:
            result = match_results.get(sensor_id, {})
            scan = result.get("scan", np.array([], dtype=np.int64))
            overall = result.get("overall", np.array([], dtype=np.float16))
            precision = result.get("precision", np.array([], dtype=np.float16))
            recall = result.get("recall", np.array([], dtype=np.float16))
            f1 = result.get("f1", np.array([], dtype=np.float16))
            accuracy = result.get("accuracy", np.array([], dtype=np.float16))

            radar_name = self._business.FRIENDLY.get(sensor_id, sensor_id)
            radar_plot_data[radar_name] = (scan, overall)
            kpi_rows.append(
                [
                    radar_name,
                    str(len(scan)),
                    f"{self._business._avg(overall):.2f}",
                    f"{self._business._avg(precision):.2f}",
                    f"{self._business._avg(recall):.2f}",
                    f"{self._business._avg(f1):.2f}",
                    f"{self._business._avg(accuracy):.2f}",
                ]
            )

        kpi_table = self._html.stats_table(
            "KPI Summary — Sensors",
            [
                "Sensor",
                "Aligned Scans",
                "Overall",
                "Precision",
                "Recall",
                "F1",
                "Accuracy",
            ],
            kpi_rows,
        )
        kpi_plot = self._html.match_all_radars_plot(
            radar_plot_data, "Overall Match % Across Sensors"
        )
        sensor_tabs["KPI"] = self._html.build_kpi_tab(kpi_table, kpi_plot)

        html = self._html.build_tabbed_html(sensor_tabs, title, summary_html)

        signals = {}
        for sid, store in in_stores.items():
            signals[f"input/{sid}"] = store.get_detection_signal_names()
        for sid, store in out_stores.items():
            signals[f"output/{sid}"] = store.get_detection_signal_names()

        return {
            "html": html,
            "signals": signals,
            "storages": {"input": in_stores, "output": out_stores},
        }

    def _build_index(self, reports: List[Tuple[str, str]]) -> str:
        items = "\n".join(
            f'<li><a href="{fname}">{name}</a></li>' for name, fname in reports
        )
        return "\n".join(
            [
                "<!DOCTYPE html>",
                "<html><head>",
                '<meta charset="utf-8"/>',
                '<meta name="viewport" content="width=device-width, initial-scale=1"/>',
                "<title>PCAN KPI — Index</title>",
                "<style>",
                "body{font-family:Segoe UI,Arial,sans-serif;margin:0;padding:16px;background:#f5f6fa;}",
                "h1{margin:0 0 12px 0;color:#2c3e50;}",
                "ul{margin:0;padding-left:18px;}",
                "li{margin:6px 0;}",
                "a{color:#3498db;text-decoration:none;}",
                "a:hover{text-decoration:underline;}",
                "</style>",
                "</head><body>",
                "<h1>PCAN KPI Reports</h1>",
                f"<ul>{items}</ul>",
                "</body></html>",
            ]
        )


def _resolve_config_path(cli_arg: Optional[str]) -> str:
    if cli_arg:
        return cli_arg

    local = Path("kpi.json")
    if local.exists():
        return str(local)

    # PyInstaller one-dir: data files usually live under sys._MEIPASS (e.g., dist/can_kpi/_internal)
    meipass = getattr(sys, "_MEIPASS", None)
    if meipass:
        bundled = Path(meipass) / "kpi.json"
        if bundled.exists():
            return str(bundled)

    # Dev fallback: alongside this source file.
    here = Path(__file__).resolve().parent
    dev = here / "kpi.json"
    if dev.exists():
        return str(dev)

    return "kpi.json"


def _parse_args(argv: list[str]) -> tuple[str, Path]:
    """Return (config_path, output_dir)."""
    config = _resolve_config_path(
        argv[1] if len(argv) > 1 and argv[1].lower().endswith(".json") else None
    )
    out_dir = Path("out_html")

    if len(argv) == 2 and not argv[1].lower().endswith(".json"):
        out_dir = Path(argv[1])
    elif len(argv) >= 3:
        config = argv[1]
        out_dir = Path(argv[2])

    return config, out_dir


if __name__ == "__main__":
    cfg, out_dir = _parse_args(sys.argv)
    KpiMain().run(cfg, out_dir)
