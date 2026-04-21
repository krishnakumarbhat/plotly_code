from __future__ import annotations

import json
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Any


class DaConfigLoader:
    def __init__(self, workspace_root: Path) -> None:
        self._workspace_root = workspace_root

    def _resolve_path(self, value: str | None) -> Path | None:
        if not value:
            return None
        path = Path(value).expanduser()
        if path.is_absolute():
            return path
        return (self._workspace_root / path).resolve()

    def _read_json_file(self, path: Path | None, fallback: dict[str, Any]) -> dict[str, Any]:
        if not path or not path.exists():
            return dict(fallback)
        return json.loads(path.read_text(encoding="utf-8"))

    def _load_xml_values(self, xml_path: Path | None) -> dict[str, str]:
        if not xml_path or not xml_path.exists():
            return {}
        root = ET.parse(xml_path).getroot()
        source_node = root.find("HDF_SOURCE_SELECTION")
        file_node = root.find("HDF_FILE")
        return {
            "hdf_source_selection": (source_node.text or "").strip().upper() if source_node is not None else "",
            "hdf_file_mode": (file_node.text or "").strip().upper() if file_node is not None else "",
        }

    def _build_pairs(self, inputs_json_path: Path | None) -> list[dict[str, str]]:
        if not inputs_json_path or not inputs_json_path.exists():
            return []
        payload = json.loads(inputs_json_path.read_text(encoding="utf-8"))
        input_list = payload.get("INPUT_HDF", []) or []
        output_list = payload.get("OUTPUT_HDF", []) or []
        pair_count = min(len(input_list), len(output_list))
        pairs: list[dict[str, str]] = []
        for index in range(pair_count):
            pairs.append({"input": str(input_list[index]), "output": str(output_list[index])})
        return pairs

    def load(
        self,
        config_path: str | None,
        output_dir: str,
        source_override: str | None,
        run_int_override: bool | None,
    ) -> dict[str, Any]:
        config_file = self._resolve_path(config_path) if config_path else (self._workspace_root / "config.json")
        defaults = {
            "hdf_source_selection": "AUTO",
            "xml_config": "ConfigInteractivePlots.xml",
            "inputs_json": "inputs.json",
            "run_int": False,
        }
        config_payload = self._read_json_file(config_file, defaults)

        xml_path = self._resolve_path(config_payload.get("xml_config", "ConfigInteractivePlots.xml"))
        inputs_json_path = self._resolve_path(config_payload.get("inputs_json", "inputs.json"))
        xml_values = self._load_xml_values(xml_path)

        selected_source = (source_override or config_payload.get("hdf_source_selection") or "AUTO").upper()
        if selected_source == "AUTO":
            selected_source = xml_values.get("hdf_source_selection") or "BORDNET"

        run_int = bool(config_payload.get("run_int", False))
        if run_int_override is not None:
            run_int = run_int_override

        resolved_output = self._resolve_path(output_dir)
        pairs = self._build_pairs(inputs_json_path)

        return {
            "workspace_root": str(self._workspace_root),
            "config_file": str(config_file) if config_file else None,
            "output_dir": str(resolved_output) if resolved_output else str(self._workspace_root / "out_html"),
            "xml_config": str(xml_path) if xml_path else None,
            "inputs_json": str(inputs_json_path) if inputs_json_path else None,
            "hdf_source_selection": selected_source,
            "hdf_file_mode": xml_values.get("hdf_file_mode", ""),
            "run_int": run_int,
            "pairs": pairs,
        }
