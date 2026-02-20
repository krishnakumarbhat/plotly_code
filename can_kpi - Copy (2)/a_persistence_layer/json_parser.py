import json
import logging
from pathlib import Path
from typing import Dict, List

logger = logging.getLogger(__name__)


class KpiJsonParser:
    """Parses `kpi.json` and validates referenced HDF paths."""

    REQUIRED_KEYS = ("INPUT_HDF", "OUTPUT_HDF")

    def parse(self, path: str) -> Dict:
        base_dir = Path(path).resolve().parent
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)

        for key in self.REQUIRED_KEYS:
            if key not in data:
                logger.warning(f"Key '{key}' not found in {path}")
                data[key] = []

        for key in self.REQUIRED_KEYS:
            data[key] = self._existing_paths(data.get(key, []), key, base_dir)

        return data

    def _existing_paths(self, paths: List[str], key: str, base_dir: Path) -> List[str]:
        valid: List[str] = []
        for p in paths:
            if not p:
                continue
            pp = Path(p)
            if not pp.is_absolute():
                pp = (base_dir / pp).resolve()
            if pp.exists():
                valid.append(str(pp))
            else:
                logger.warning(f"{key} file not found: {p}")
        return valid
