"""CAN Radar KPI — main entry point."""

import logging
import sys
from pathlib import Path
from typing import Optional

from c_business_layer.kpi_business import KpiBusiness

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s  %(levelname)-8s  %(name)s  %(message)s",
)
logger = logging.getLogger(__name__)


class KpiMain:
    def __init__(self, business: Optional[KpiBusiness] = None):
        self._business = business or KpiBusiness()

    def run(self, config_path: str = "kpi.json") -> Path:
        logger.info(f"Config: {config_path}")
        result = self._business.run(config_path)
        out_path = Path("kpi_report.html")
        out_path.write_text(result["html"], encoding="utf-8")
        logger.info(f"Wrote report to {out_path.resolve()}")
        for key, sigs in result.get("signals", {}).items():
            logger.info(f"  {key}: {len(sigs)} signals")
        return out_path


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


if __name__ == "__main__":
    cfg = _resolve_config_path(sys.argv[1] if len(sys.argv) > 1 else None)
    KpiMain().run(cfg)
