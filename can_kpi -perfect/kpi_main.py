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

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s  %(levelname)-8s  %(name)s  %(message)s",
)
logger = logging.getLogger(__name__)


class KpiMain:
    def __init__(self, business: Optional[KpiBusiness] = None):
        self._json = KpiJsonParser()
        self._business = business or KpiBusiness()

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
                result = self._business.run_pair(in_path, out_path, title=title)
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
