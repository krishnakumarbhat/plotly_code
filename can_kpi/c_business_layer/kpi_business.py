"""Business orchestration — builds tabbed scatter report."""

import logging
import math
from collections import OrderedDict
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import numpy as np

from a_persistence_layer.json_parser import KpiJsonParser
from a_persistence_layer.kpi_hdf_parser import KpiHdfParser
from b_data_storage.can_kpi_store import CANKpiDataStore
from d_presentation_layer.kpi_html_gen import KpiHtmlGen

logger = logging.getLogger(__name__)


class KpiBusiness:
    """Runs the full pipeline producing an HTML report with sensor tabs."""

    SCATTER_SIGNALS = [
        "DET_RANGE",
        "DET_RANGE_VELOCITY",
        "DET_AZIMUTH",
        "DET_ELEVATION",
        "DET_RCS",
        "DET_SNR",
    ]

    MATCH_SIGNALS = ["DET_RANGE", "DET_RANGE_VELOCITY", "DET_AZIMUTH", "DET_ELEVATION"]

    MATCH_TOLERANCES = {
        "DET_RANGE": 0.01,
        "DET_RANGE_VELOCITY": 0.01,
        "DET_AZIMUTH": 0.01,
        "DET_ELEVATION": 0.01,
    }

    SENSOR_ORDER = ["CEER_FL", "CEER_FLR", "CEER_FR", "CEER_RL", "CEER_RR"]

    FRIENDLY = {
        "CEER_FL": "SRR / FL",
        "CEER_FLR": "SRR / FLR",
        "CEER_FR": "SRR / FR",
        "CEER_RL": "SRR / RL",
        "CEER_RR": "SRR / RR",
    }

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
        # Legacy behavior: build a single report from the first pair.
        # Multi-log reporting is handled in the entrypoint (kpi_main.py).
        in_path = input_paths[0] if input_paths else None
        out_path = output_paths[0] if output_paths else None
        return self.run_pair(in_path, out_path, title="PCAN Detection KPI")

    def run_pair(
        self,
        input_hdf: Optional[str],
        output_hdf: Optional[str],
        title: str = "PCAN Detection KPI",
    ) -> Dict[str, Any]:
        """Build a report for a single (input_hdf, output_hdf) pair."""
        if not input_hdf and not output_hdf:
            raise ValueError("At least one of input_hdf/output_hdf must be provided")

        store = CANKpiDataStore()
        in_parsed = self._hdf.parse_file(input_hdf) if input_hdf else {}
        out_parsed = self._hdf.parse_file(output_hdf) if output_hdf else {}
        store.add_parsed_data("input", in_parsed)
        store.add_parsed_data("output", out_parsed)

        all_sensors = sorted(
            set(store.sensor_ids("input")) | set(store.sensor_ids("output")),
            key=lambda s: self.SENSOR_ORDER.index(s) if s in self.SENSOR_ORDER else 99,
        )

        # ── Compute match percentages for all sensors ──────────────────
        match_results: Dict[
            str, Tuple
        ] = {}  # {sensor_id: (common_scan, overall_pct, per_param_pct)}
        for sensor_id in all_sensors:
            match_results[sensor_id] = self._compute_match_pct(store, sensor_id)

        summary_html = self._build_summary_tables(
            store,
            all_sensors,
            [input_hdf] if input_hdf else [],
            [output_hdf] if output_hdf else [],
        )

        sensor_tabs: OrderedDict[str, str] = OrderedDict()

        for sensor_id in all_sensors:
            tab_label = self.FRIENDLY.get(sensor_id, sensor_id)
            content = self._build_sensor_tab(
                store, sensor_id, tab_label, match_results.get(sensor_id)
            )
            sensor_tabs[tab_label] = content

        html = self._html.build_tabbed_html(sensor_tabs, title, summary_html)

        signals: Dict[str, List[str]] = {}
        for lbl in store.labels():
            for sid in store.sensor_ids(lbl):
                signals[f"{lbl}/{sid}"] = store.get_all_signal_names(lbl, sid)

        return {"html": html, "signals": signals, "store": store}

    def _build_sensor_tab(
        self,
        store: CANKpiDataStore,
        sensor_id: str,
        friendly: str,
        match_result: Optional[Tuple] = None,
    ) -> str:
        parts: List[str] = []

        # ── Match % plot (overall) ─────────────────────────────────────
        if match_result is not None:
            common_scan, overall_pct, per_param_pct = match_result
            if len(common_scan) > 0:
                parts.append(
                    f"<h2>{friendly} — Detection Match % (Input vs Output)</h2>"
                )
                pct_dict = {"Overall": overall_pct}
                div = self._html.match_line_plot(
                    common_scan, pct_dict, f"{friendly} — Detection Match %"
                )
                parts.append(f'<div class="plot-card">{div}</div>')

        # ── Raw scatter plots ──────────────────────────────────────────
        parts.append(f"<h2>{friendly} — Detection Scatter Plots (Input vs Output)</h2>")

        available_signals = set()
        for lbl in ["input", "output"]:
            available_signals.update(store.get_all_signal_names(lbl, sensor_id))

        plot_signals = [s for s in self.SCATTER_SIGNALS if s in available_signals]

        if not plot_signals:
            parts.append(
                f"<p><em>No detection signals available for {friendly}.</em></p>"
            )
            return "\n".join(parts)

        parts.append('<div class="plots-grid">')
        for sig_name in plot_signals:
            in_scan, in_2d = store.get_detection_2d("input", sensor_id, sig_name)
            out_scan, out_2d = store.get_detection_2d("output", sensor_id, sig_name)
            if in_2d.size == 0 and out_2d.size == 0:
                continue
            div = self._html.input_output_scatter(
                in_scan, in_2d, out_scan, out_2d, sig_name, friendly
            )
            parts.append(f'<div class="plot-card">{div}</div>')
        parts.append("</div>")

        return "\n".join(parts)

    def _compute_match_pct(
        self, store: CANKpiDataStore, sensor_id: str
    ) -> Tuple[np.ndarray, np.ndarray, Dict[str, np.ndarray]]:
        """
        Compare input vs output detection-by-detection per scan index.

                For each common scan index:
                - Detection *i* in input is compared with detection *i* in output.
                - A detection counts as "matched" when **all four** parameter diffs
                    (range, range_velocity, azimuth, elevation) are within tolerance.
                - Values are rounded to 2 decimal places before comparison.
                - Only detections 1..N are considered, where N comes from the per-scan
                    header signal `HED_NUM_OF_VALID_DETECTIONS`.
                - Percentage = matched_count / N * 100 (if input/output N differs, the
                    comparison uses `min(N_in, N_out)` as a safe fallback).

        Returns
        -------
        common_scan : ndarray
        overall_pct : ndarray (float16)
        per_param_pct : dict {signal_name: ndarray (float16)}
        """
        in_scan = store.get_scan_index("input", sensor_id)
        out_scan = store.get_scan_index("output", sensor_id)
        common = (
            np.intersect1d(in_scan, out_scan)
            if len(in_scan) and len(out_scan)
            else np.array([], dtype=np.int64)
        )
        n = len(common)
        empty_params = {s: np.array([], dtype=np.float16) for s in self.MATCH_SIGNALS}
        if n == 0:
            return common, np.array([], dtype=np.float16), empty_params

        # Row-index maps for the full arrays
        in_map = {int(v): i for i, v in enumerate(in_scan)}
        out_map = {int(v): i for i, v in enumerate(out_scan)}
        in_rows = np.array([in_map[int(s)] for s in common])
        out_rows = np.array([out_map[int(s)] for s in common])

        # Requirement: use header signal HED_NUM_OF_VALID_DETECTIONS (per scan)
        # as the denominator and only consider detections 1..N for that scan.
        in_hdr = store.get_header_signals("input", sensor_id).get(
            "HED_NUM_OF_VALID_DETECTIONS"
        )
        out_hdr = store.get_header_signals("output", sensor_id).get(
            "HED_NUM_OF_VALID_DETECTIONS"
        )

        if not (isinstance(in_hdr, np.ndarray) and in_hdr.size > 0):
            return common, np.zeros(n, dtype=np.float16), empty_params
        if not (isinstance(out_hdr, np.ndarray) and out_hdr.size > 0):
            return common, np.zeros(n, dtype=np.float16), empty_params

        in_cnt = np.rint(in_hdr[in_rows]).astype(np.int32)
        out_cnt = np.rint(out_hdr[out_rows]).astype(np.int32)
        in_cnt = np.clip(in_cnt, 0, None)
        out_cnt = np.clip(out_cnt, 0, None)

        # Input/output counts are expected to match; if not, only compare the common portion.
        denom_cnt = np.minimum(in_cnt, out_cnt).astype(np.int32)
        if np.any(in_cnt != out_cnt):
            logger.warning(
                f"{sensor_id}: HED_NUM_OF_VALID_DETECTIONS differs between input/output "
                f"for {int(np.sum(in_cnt != out_cnt))} scans; using min(in,out)"
            )

        # Pull the per-detection signal dicts once (avoids building large 2-D arrays).
        in_sig: Dict[str, Dict[int, np.ndarray]] = {}
        out_sig: Dict[str, Dict[int, np.ndarray]] = {}
        for sig in self.MATCH_SIGNALS:
            in_sig[sig] = store.get_detection_signal("input", sensor_id, sig) or {}
            out_sig[sig] = store.get_detection_signal("output", sensor_id, sig) or {}

        overall_pct = np.zeros(n, dtype=np.float16)
        per_param_pct: Dict[str, np.ndarray] = {
            sig: np.zeros(n, dtype=np.float16) for sig in self.MATCH_SIGNALS
        }

        for i in range(n):
            n_det = int(denom_cnt[i])
            if n_det <= 0:
                continue

            in_r = int(in_rows[i])
            out_r = int(out_rows[i])

            matched_all = 0
            matched_param = {sig: 0 for sig in self.MATCH_SIGNALS}

            for det_idx in range(1, n_det + 1):
                # Range validity gate
                rin_arr = in_sig["DET_RANGE"].get(det_idx)
                rout_arr = out_sig["DET_RANGE"].get(det_idx)
                if rin_arr is None or rout_arr is None:
                    continue
                if in_r >= len(rin_arr) or out_r >= len(rout_arr):
                    continue

                rin = float(rin_arr[in_r])
                rout = float(rout_arr[out_r])
                if math.isnan(rin) or math.isnan(rout) or rin == 0.0 or rout == 0.0:
                    continue

                rin = round(rin, 2)
                rout = round(rout, 2)

                det_all_ok = True
                for sig in self.MATCH_SIGNALS:
                    ain_arr = in_sig[sig].get(det_idx)
                    aout_arr = out_sig[sig].get(det_idx)
                    if ain_arr is None or aout_arr is None:
                        det_all_ok = False
                        continue
                    if in_r >= len(ain_arr) or out_r >= len(aout_arr):
                        det_all_ok = False
                        continue

                    ain = float(ain_arr[in_r])
                    aout = float(aout_arr[out_r])
                    if math.isnan(ain) or math.isnan(aout):
                        det_all_ok = False
                        continue

                    ain = round(ain, 2)
                    aout = round(aout, 2)
                    if abs(ain - aout) <= float(self.MATCH_TOLERANCES[sig]):
                        matched_param[sig] += 1
                    else:
                        det_all_ok = False

                if det_all_ok:
                    matched_all += 1

            overall_pct[i] = np.float16(round(100.0 * matched_all / n_det, 2))
            for sig in self.MATCH_SIGNALS:
                per_param_pct[sig][i] = np.float16(
                    round(100.0 * matched_param[sig] / n_det, 2)
                )

        return common, overall_pct, per_param_pct

    def _build_all_radars_match(self, match_results: Dict[str, Tuple]) -> str:
        """Build 'All Radars' combined match % plots shown in the summary area."""
        radar_overall: Dict[str, tuple] = {}
        for sensor_id, (common, overall_pct, _) in match_results.items():
            if len(common) == 0:
                continue
            friendly = self.FRIENDLY.get(sensor_id, sensor_id)
            radar_overall[friendly] = (common, overall_pct)

        if not radar_overall:
            return ""

        parts: List[str] = []
        parts.append("<h2>Detection Match % — All Radars (Overall)</h2>")
        div = self._html.match_all_radars_plot(
            radar_overall, "Detection Match % — All Radars"
        )
        parts.append(f'<div class="plot-card">{div}</div>')

        # Per-parameter combined plots
        param_friendly = {
            "DET_RANGE": "Range",
            "DET_RANGE_VELOCITY": "Range Velocity",
            "DET_AZIMUTH": "Azimuth",
            "DET_ELEVATION": "Elevation",
        }
        for sig in self.MATCH_SIGNALS:
            radar_param: Dict[str, tuple] = {}
            for sensor_id, (common, _, per_param) in match_results.items():
                if len(common) == 0 or sig not in per_param:
                    continue
                friendly = self.FRIENDLY.get(sensor_id, sensor_id)
                radar_param[friendly] = (common, per_param[sig])

            if radar_param:
                pf = param_friendly.get(sig, sig)
                parts.append(f"<h2>{pf} Match % — All Radars</h2>")
                div = self._html.match_all_radars_plot(
                    radar_param, f"{pf} Match % — All Radars"
                )
                parts.append(f'<div class="plot-card">{div}</div>')

        return "\n".join(parts)

    def _build_summary_tables(
        self,
        store: CANKpiDataStore,
        all_sensors: List[str],
        input_paths: List[str],
        output_paths: List[str],
    ) -> str:
        headers = [
            "Sensor",
            "Scans (Input)",
            "Scans (Output)",
            "Common Scans",
            "Scan Match %",
            "Det Signals (Input)",
            "Det Signals (Output)",
        ]
        rows: List[List[str]] = []
        for sensor_id in all_sensors:
            friendly = self.FRIENDLY.get(sensor_id, sensor_id)
            in_scan = store.get_scan_index("input", sensor_id)
            out_scan = store.get_scan_index("output", sensor_id)
            common = (
                np.intersect1d(in_scan, out_scan)
                if len(in_scan) and len(out_scan)
                else np.array([])
            )
            pct = self._pct(len(common), max(len(in_scan), len(out_scan)))
            in_sigs = len(store.get_all_signal_names("input", sensor_id))
            out_sigs = len(store.get_all_signal_names("output", sensor_id))
            rows.append(
                [
                    friendly,
                    str(len(in_scan)),
                    str(len(out_scan)),
                    str(len(common)),
                    f"{pct:.2f}" if not math.isnan(pct) else "NA",
                    str(in_sigs),
                    str(out_sigs),
                ]
            )
        return self._html.stats_table("Overview — All Sensors", headers, rows)

    def _pct(self, num: int, den: int) -> float:
        if den <= 0:
            return float("nan")
        return 100.0 * float(num) / float(den)
