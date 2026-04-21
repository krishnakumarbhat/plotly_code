from __future__ import annotations

import json
from pathlib import Path
from typing import Any


class HtmlReportRepository:
    def write_pair_report(
        self,
        output_dir: Path,
        project_name: str,
        pair_index: int,
        input_file: str,
        output_file: str,
        metrics: dict[str, Any],
    ) -> Path:
        output_dir.mkdir(parents=True, exist_ok=True)
        report_path = output_dir / f"{project_name}_pair_{pair_index + 1}.html"

        rows = [
            ("Input file", input_file),
            ("Output file", output_file),
            ("Input sensors", ", ".join(metrics.get("input_sensors", []))),
            ("Output sensors", ", ".join(metrics.get("output_sensors", []))),
            ("Input dataset count", str(metrics.get("input_dataset_count", 0))),
            ("Output dataset count", str(metrics.get("output_dataset_count", 0))),
            ("Common dataset count", str(metrics.get("common_dataset_count", 0))),
            ("Input-only dataset count", str(metrics.get("input_only_count", 0))),
            ("Output-only dataset count", str(metrics.get("output_only_count", 0))),
            ("Shape match %", f"{metrics.get('shape_match_pct', 0.0):.2f}"),
            ("Dtype match %", f"{metrics.get('dtype_match_pct', 0.0):.2f}"),
        ]

        body = "\n".join([f"<tr><th>{key}</th><td>{value}</td></tr>" for key, value in rows])
        html = f"""
<!DOCTYPE html>
<html>
<head>
  <meta charset=\"utf-8\" />
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />
  <title>{project_name.upper()} KPI Pair {pair_index + 1}</title>
  <style>
    body {{ font-family: Segoe UI, Arial, sans-serif; margin: 20px; background: #f4f6f8; }}
    .card {{ background: #fff; padding: 16px; border-radius: 10px; border: 1px solid #dde3ea; max-width: 980px; }}
    table {{ width: 100%; border-collapse: collapse; }}
    th, td {{ text-align: left; padding: 10px; border-bottom: 1px solid #ecf0f4; }}
    th {{ width: 280px; color: #34495e; }}
    h1 {{ margin-top: 0; }}
  </style>
</head>
<body>
  <div class=\"card\">
    <h1>{project_name.upper()} KPI - Pair {pair_index + 1}</h1>
    <table>{body}</table>
  </div>
</body>
</html>
"""
        report_path.write_text(html, encoding="utf-8")
        return report_path

    def write_index(self, output_dir: Path, title: str, report_paths: list[Path]) -> Path:
        output_dir.mkdir(parents=True, exist_ok=True)
        index_path = output_dir / "index.html"
        links = "\n".join(
            [f'<li><a href="{item.name}">{item.name}</a></li>' for item in report_paths]
        )
        html = f"""
<!DOCTYPE html>
<html>
<head>
  <meta charset=\"utf-8\" />
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />
  <title>{title}</title>
</head>
<body>
  <h1>{title}</h1>
  <ul>{links}</ul>
</body>
</html>
"""
        index_path.write_text(html, encoding="utf-8")
        return index_path

    def write_json(self, file_path: Path, payload: dict[str, Any]) -> Path:
        file_path.parent.mkdir(parents=True, exist_ok=True)
        file_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
        return file_path
