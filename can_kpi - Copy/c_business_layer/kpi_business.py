"""Business orchestration and KPI computations."""

import logging
import math
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import numpy as np

from a_persistence_layer.json_parser import KpiJsonParser
from a_persistence_layer.kpi_hdf_parser import KpiHdfParser
from b_data_storage.can_kpi_store import CANKpiDataStore
from d_presentation_layer.kpi_html_gen import KpiHtmlGen

logger = logging.getLogger(__name__)


class KpiBusiness:
    """Runs the full pipeline and produces an HTML report."""

    PRIMARY_SIGNALS = (
        "DET_AZIMUTH",
        "DET_ELEVATION",
        "DET_RANGE",
        "DET_RANGE_VELOCITY",
    )

    def __init__(
        self,
        json_parser: Optional[KpiJsonParser] = None,
        hdf_parser: Optional[KpiHdfParser] = None,
        html: Optional[KpiHtmlGen] = None,
        eps: float = 1e-6,
    ):
        self._json = json_parser or KpiJsonParser()
        self._hdf = hdf_parser or KpiHdfParser()
        self._html = html or KpiHtmlGen()
        self._eps = float(eps)

    def run(self, kpi_json_path: str) -> Dict[str, Any]:
        cfg = self._json.parse(kpi_json_path)
        input_paths = cfg.get("INPUT_HDF", [])
        output_paths = cfg.get("OUTPUT_HDF", [])
        n_logs = max(len(input_paths), len(output_paths))
        if n_logs == 0:
            raise ValueError("No INPUT_HDF/OUTPUT_HDF entries found")

        stats_by_sensor: Dict[str, Dict[str, List[float]]] = {}
        table_by_sensor: Dict[str, List[List[str]]] = {}

        # Keep one store for optional detailed plots (last parsed pair)
        last_store: Optional[CANKpiDataStore] = None
        last_signals: Dict[str, List[str]] = {}

        for i in range(n_logs):
            in_path = input_paths[i] if i < len(input_paths) else None
            out_path = output_paths[i] if i < len(output_paths) else None
            in_parsed = self._hdf.parse_file(in_path) if in_path else {}
            out_parsed = self._hdf.parse_file(out_path) if out_path else {}

            store = CANKpiDataStore()
            store.add_parsed_data("input", in_parsed)
            store.add_parsed_data("output", out_parsed)
            last_store = store

            for sensor_id in sorted(
                set(store.sensor_ids("input")) | set(store.sensor_ids("output"))
            ):
                friendly = (
                    store.get_sensor("input", sensor_id)
                    or store.get_sensor("output", sensor_id)
                    or {}
                ).get("friendly_name", sensor_id)
                series_name = self._series_name(sensor_id, friendly)

                scan_in = store.get_scan_index("input", sensor_id)
                scan_out = store.get_scan_index("output", sensor_id)
                common = (
                    np.intersect1d(scan_in, scan_out)
                    if len(scan_in) and len(scan_out)
                    else np.array([])
                )

                scan_match = self._pct(len(common), max(len(scan_in), len(scan_out)))
                det_match, det_matches, det_total = self._detections_match(
                    store, sensor_id, common
                )
                az_match, az_m, az_t = self._alignment_match(
                    store, sensor_id, common, "az"
                )
                el_match, el_m, el_t = self._alignment_match(
                    store, sensor_id, common, "el"
                )

                stats_by_sensor.setdefault(
                    series_name, {"det": [], "scan": [], "az": [], "el": []}
                )
                stats_by_sensor[series_name]["det"].append(det_match)
                stats_by_sensor[series_name]["scan"].append(scan_match)
                stats_by_sensor[series_name]["az"].append(az_match)
                stats_by_sensor[series_name]["el"].append(el_match)

                row = [
                    Path(in_path or out_path or f"log_{i}").name,
                    str(len(scan_in)),
                    str(len(scan_out)),
                    str(len(common)),
                    f"{scan_match:.2f}" if not math.isnan(scan_match) else "NA",
                    f"({det_matches}/{det_total}) {det_match:.2f}"
                    if det_total
                    else "(0/0) NA",
                    f"({el_m}/{el_t}) {el_match:.2f}" if el_t else "(0/0) NA",
                    f"({az_m}/{az_t}) {az_match:.2f}" if az_t else "(0/0) NA",
                ]
                table_by_sensor.setdefault(series_name, []).append(row)

            last_signals = self._collect_signals(store)

        # ── Build consolidated report (similar to your reference HTML) ──
        x = list(range(1, n_logs + 1))
        divs: List[str] = []
        det_plot = self._html.line_multi(
            x,
            {k: v["det"] for k, v in stats_by_sensor.items()},
            "Detections matched (%) — All Radars",
            "Log Index",
            "Percentage",
        )
        divs.append(f"<h2>Detections matched (%) — All Radars</h2>{det_plot}")

        scan_plot = self._html.line_multi(
            x,
            {k: v["scan"] for k, v in stats_by_sensor.items()},
            "Timestamp-Scanindex match (%) — All Radars",
            "Log Index",
            "Percentage",
        )
        divs.append(f"<h2>Timestamp-Scanindex match (%) — All Radars</h2>{scan_plot}")

        el_plot = self._html.line_multi(
            x,
            {k: v["el"] for k, v in stats_by_sensor.items()},
            "Alignment Elevation match (%) — All Radars",
            "Log Index",
            "Percentage",
        )
        divs.append(f"<h2>Alignment Elevation match (%) — All Radars</h2>{el_plot}")

        az_plot = self._html.line_multi(
            x,
            {k: v["az"] for k, v in stats_by_sensor.items()},
            "Alignment Azimuth match (%) — All Radars",
            "Log Index",
            "Percentage",
        )
        divs.append(f"<h2>Alignment Azimuth match (%) — All Radars</h2>{az_plot}")

        divs.append("<h2>Data Tables</h2>")
        headers = [
            "Filename",
            "decoded_scanindex_input",
            "decoded_scanindex_output",
            "scanindex_used_for_comparison",
            "timestamp-scanindex match (%)",
            "dets parameter match (matches/total) %",
            "alignment elevation match (matches/total) %",
            "alignment azimuth match (matches/total) %",
        ]
        tables_html = ['<div class="tables-container">']
        for sensor_name, rows in table_by_sensor.items():
            tables_html.append('<div class="table-wrapper">')
            tables_html.append(
                self._html.stats_table(
                    f"{sensor_name} - Statistics Summary", headers, rows
                )
            )
            tables_html.append("</div>")
        tables_html.append("</div>")
        divs.append("".join(tables_html))

        # Optional: a small detailed section for the last log (keeps output manageable)
        if last_store is not None:
            divs.extend(self._detail_section(last_store))

        html = self._html.wrap_html(divs, title="PCAN Detection KPI")
        return {"html": html, "signals": last_signals, "store": last_store}

    def _detail_section(self, store: CANKpiDataStore) -> List[str]:
        out: List[str] = []
        out.append("<h2>Sample Detailed Plots (last log)</h2>")
        for sensor_id in sorted(
            set(store.sensor_ids("input")) | set(store.sensor_ids("output"))
        ):
            sd = store.get_sensor("input", sensor_id) or store.get_sensor(
                "output", sensor_id
            )
            if not sd:
                continue
            friendly = sd.get("friendly_name", sensor_id)
            out.append(f"<h3>{self._series_name(sensor_id, friendly)}</h3>")

            scan = store.get_scan_index("input", sensor_id)
            hdr = store.get_header_signals("input", sensor_id)
            num_valid = hdr.get("HED_NUM_OF_VALID_DETECTIONS") if hdr else None
            if num_valid is not None and len(scan) == len(num_valid):
                out.append(self._html.header_num_detections(scan, num_valid, friendly))

            common = np.intersect1d(
                store.get_scan_index("input", sensor_id),
                store.get_scan_index("output", sensor_id),
            )
            if len(common):
                # Compare DET_RANGE det#1 as a quick sanity plot
                in_scan, in2d = store.get_detection_2d("input", sensor_id, "DET_RANGE")
                out_scan, out2d = store.get_detection_2d(
                    "output", sensor_id, "DET_RANGE"
                )
                if in2d.size and out2d.size:
                    # align both to common
                    in_map = {int(v): i for i, v in enumerate(in_scan)}
                    out_map = {int(v): i for i, v in enumerate(out_scan)}
                    in_common = np.array(
                        [in2d[in_map[int(s)]] for s in common if int(s) in in_map]
                    )
                    out_common = np.array(
                        [out2d[out_map[int(s)]] for s in common if int(s) in out_map]
                    )
                    if len(in_common) and len(out_common):
                        out.append(
                            self._html.comparison_scatter(
                                common,
                                in_common,
                                out_common,
                                "DET_RANGE",
                                friendly,
                                det_idx=1,
                            )
                        )

        return out

    def _detections_match(
        self, store: CANKpiDataStore, sensor_id: str, common: np.ndarray
    ) -> Tuple[float, int, int]:
        if len(common) == 0:
            return float("nan"), 0, 0
        matches = 0
        total = 0
        for sig in self.PRIMARY_SIGNALS:
            cmp = store.compare_detection_signal(sensor_id, sig)
            if (
                cmp.get("common_scan_index") is None
                or len(cmp["common_scan_index"]) == 0
            ):
                continue
            # build on common for this sig
            diff = cmp.get("diff_2d")
            if diff is None or diff.size == 0:
                continue
            valid = ~np.isnan(diff)
            total += int(valid.sum())
            matches += int((np.abs(diff) <= self._eps)[valid].sum())
        return (self._pct(matches, total), matches, total)

    def _alignment_match(
        self, store: CANKpiDataStore, sensor_id: str, common: np.ndarray, which: str
    ) -> Tuple[float, int, int]:
        if len(common) == 0:
            return float("nan"), 0, 0
        in_sd = store.get_sensor("input", sensor_id)
        out_sd = store.get_sensor("output", sensor_id)
        if not in_sd or not out_sd:
            return float("nan"), 0, 0

        def pick(sd: Dict[str, Any]) -> Optional[np.ndarray]:
            a = sd.get("alignment_signals", {})
            if which == "az":
                return a.get("STS_AACurrentAzimuth", a.get("STS_AlignmentAzimuth"))
            return a.get("STS_AACurrentElevation", a.get("STS_AlignmentElevation"))

        in_arr, out_arr = pick(in_sd), pick(out_sd)
        if in_arr is None or out_arr is None:
            return float("nan"), 0, 0

        in_scan = in_sd.get("scan_index", np.array([]))
        out_scan = out_sd.get("scan_index", np.array([]))
        in_row = {int(v): i for i, v in enumerate(in_scan)}
        out_row = {int(v): i for i, v in enumerate(out_scan)}

        diffs = []
        for sv in common:
            ri, ro = in_row.get(int(sv)), out_row.get(int(sv))
            if ri is None or ro is None or ri >= len(in_arr) or ro >= len(out_arr):
                continue
            vi, vo = in_arr[ri], out_arr[ro]
            if np.isnan(vi) or np.isnan(vo):
                continue
            diffs.append(vi - vo)
        if not diffs:
            return float("nan"), 0, 0

        diffs = np.array(diffs)
        tot = int(len(diffs))
        mat = int((np.abs(diffs) <= self._eps).sum())
        return self._pct(mat, tot), mat, tot

    def _collect_signals(self, store: CANKpiDataStore) -> Dict[str, List[str]]:
        out: Dict[str, List[str]] = {}
        for lbl in store.labels():
            for sid in store.sensor_ids(lbl):
                out[f"{lbl}/{sid}"] = store.get_all_signal_names(lbl, sid)
        return out

    def _pct(self, num: int, den: int) -> float:
        if den <= 0:
            return float("nan")
        return 100.0 * float(num) / float(den)

    def _series_name(self, sensor_id: str, friendly: str) -> str:
        if "FLR" in friendly or sensor_id.endswith("FLR"):
            return "FLR"
        if "SRR_FL" in friendly or sensor_id.endswith("FL"):
            return "SRR/FL"
        if "SRR_FR" in friendly or sensor_id.endswith("FR"):
            return "SRR/FR"
        if "SRR_RL" in friendly or sensor_id.endswith("RL"):
            return "SRR/RL"
        if "SRR_RR" in friendly or sensor_id.endswith("RR"):
            return "SRR/RR"
        return sensor_id
