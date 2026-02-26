"""Business orchestration — builds tabbed scatter report."""

import logging
import math
from collections import OrderedDict
from typing import Any, Dict, List, Optional, Tuple

import numpy as np

from a_persistence_layer.json_parser import KpiJsonParser
from a_persistence_layer.hdf_parser import KpiHdfParser
from d_presentation_layer.kpi_html_gen import KpiHtmlGen

logger = logging.getLogger(__name__)


class KpiBusiness:
    """Runs the full pipeline producing an HTML report with sensor tabs."""

    SCATTER_SIGNALS = [
        "DET_RANGE",
        "DET_RANGE_VELOCITY",
        "DET_AZIMUTH",
        "DET_ELEVATION",
    ]

    MATCH_SIGNALS = ["DET_RANGE", "DET_RANGE_VELOCITY", "DET_AZIMUTH", "DET_ELEVATION"]

    MATCH_TOLERANCES = {
        "DET_RANGE": 0.9,
        "DET_RANGE_VELOCITY": 0.9,
        "DET_AZIMUTH": 0.9,
        "DET_ELEVATION": 0.9,
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
        required_signals = sorted(set(self.SCATTER_SIGNALS + self.MATCH_SIGNALS))
        self._hdf = hdf_parser or KpiHdfParser(
            required_detection_signals=required_signals
        )
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

        in_parsed = self._hdf.parse_file(input_hdf) if input_hdf else {}
        out_parsed = self._hdf.parse_file(output_hdf) if output_hdf else {}
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]] = {
            "input": in_parsed,
            "output": out_parsed,
        }

        all_sensors = sorted(
            set(parsed_by_label.get("input", {}).keys())
            | set(parsed_by_label.get("output", {}).keys()),
            key=lambda s: self.SENSOR_ORDER.index(s) if s in self.SENSOR_ORDER else 99,
        )

        # ── Compute match percentages for all sensors ──────────────────
        match_results: Dict[
            str, Tuple
        ] = {}  # {sensor_id: (common_scan, overall_pct, per_param_pct)}
        for sensor_id in all_sensors:
            match_results[sensor_id] = self._compute_match_pct(
                parsed_by_label, sensor_id
            )

        summary_html = self._build_summary_tables(
            parsed_by_label,
            all_sensors,
            [input_hdf] if input_hdf else [],
            [output_hdf] if output_hdf else [],
        )

        sensor_tabs: OrderedDict[str, str] = OrderedDict()

        for sensor_id in all_sensors:
            tab_label = self.FRIENDLY.get(sensor_id, sensor_id)
            content = self._build_sensor_tab(
                parsed_by_label, sensor_id, tab_label, match_results.get(sensor_id)
            )
            sensor_tabs[tab_label] = content

        html = self._html.build_tabbed_html(sensor_tabs, title, summary_html)

        signals: Dict[str, List[str]] = {}
        for lbl in parsed_by_label:
            for sid in parsed_by_label.get(lbl, {}):
                signals[f"{lbl}/{sid}"] = self._get_all_signal_names(
                    parsed_by_label, lbl, sid
                )

        return {"html": html, "signals": signals, "data": parsed_by_label}

    def _build_sensor_tab(
        self,
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]],
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
            available_signals.update(
                self._get_all_signal_names(parsed_by_label, lbl, sensor_id)
            )

        plot_signals = [s for s in self.SCATTER_SIGNALS if s in available_signals]

        if not plot_signals:
            parts.append(
                f"<p><em>No detection signals available for {friendly}.</em></p>"
            )
            return "\n".join(parts)

        parts.append('<div class="plots-grid">')
        for sig_name in plot_signals:
            in_scan, in_2d = self._get_detection_2d(
                parsed_by_label, "input", sensor_id, sig_name
            )
            out_scan, out_2d = self._get_detection_2d(
                parsed_by_label, "output", sensor_id, sig_name
            )
            if in_2d.size == 0 and out_2d.size == 0:
                continue
            div = self._html.input_output_scatter(
                in_scan, in_2d, out_scan, out_2d, sig_name, friendly
            )
            parts.append(f'<div class="plot-card">{div}</div>')
        parts.append("</div>")

        return "\n".join(parts)

    def _compute_match_pct(
        self,
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]],
        sensor_id: str,
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
        in_scan = self._get_scan_index(parsed_by_label, "input", sensor_id)
        out_scan = self._get_scan_index(parsed_by_label, "output", sensor_id)
        common, in_rows, out_rows = self._aligned_common_rows(in_scan, out_scan)
        n = len(common)
        empty_params = {s: np.array([], dtype=np.float16) for s in self.MATCH_SIGNALS}
        if n == 0:
            return common, np.array([], dtype=np.float16), empty_params

        # Requirement: use header signal HED_NUM_OF_VALID_DETECTIONS (per scan)
        # as the denominator and only consider detections 1..N for that scan.
        in_hdr = self._get_header_signals(parsed_by_label, "input", sensor_id).get(
            "HED_NUM_OF_VALID_DETECTIONS"
        )
        out_hdr = self._get_header_signals(parsed_by_label, "output", sensor_id).get(
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
            in_raw = (
                self._get_detection_signal(parsed_by_label, "input", sensor_id, sig)
                or {}
            )
            out_raw = (
                self._get_detection_signal(parsed_by_label, "output", sensor_id, sig)
                or {}
            )
            in_sig[sig] = self._align_signal_map_to_rows(in_raw, in_rows)
            out_sig[sig] = self._align_signal_map_to_rows(out_raw, out_rows)

        in_scan_common = in_scan[in_rows]
        out_scan_common = out_scan[out_rows]

        overall_pct = np.zeros(n, dtype=np.float16)
        per_param_pct: Dict[str, np.ndarray] = {
            sig: np.zeros(n, dtype=np.float16) for sig in self.MATCH_SIGNALS
        }

        for i in range(n):
            if int(in_scan_common[i]) != int(out_scan_common[i]):
                logger.warning(
                    f"{sensor_id}: skip mismatched pair input_scan={int(in_scan_common[i])} "
                    f"output_scan={int(out_scan_common[i])}"
                )
                continue

            n_det = int(denom_cnt[i])
            if n_det <= 0:
                continue

            matched_all = 0
            matched_param = {sig: 0 for sig in self.MATCH_SIGNALS}

            for det_idx in range(1, n_det + 1):
                # Range validity gate
                rin_arr = in_sig["DET_RANGE"].get(det_idx)
                rout_arr = out_sig["DET_RANGE"].get(det_idx)
                if rin_arr is None or rout_arr is None:
                    continue
                if i >= len(rin_arr) or i >= len(rout_arr):
                    continue

                rin = float(rin_arr[i])
                rout = float(rout_arr[i])
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
                    if i >= len(ain_arr) or i >= len(aout_arr):
                        det_all_ok = False
                        continue

                    ain = float(ain_arr[i])
                    aout = float(aout_arr[i])
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

    def _align_signal_map_to_rows(
        self, signal_map: Dict[int, np.ndarray], rows: np.ndarray
    ) -> Dict[int, np.ndarray]:
        """Align a detection-index map to a specific row order.

        Result arrays are indexed by common-scan position (0..N-1),
        not by original file row index.
        """
        if not signal_map:
            return {}

        n = int(len(rows))
        out: Dict[int, np.ndarray] = {}
        for det_idx, arr in signal_map.items():
            if not isinstance(arr, np.ndarray):
                continue
            aligned = np.full(n, np.nan, dtype=np.float64)
            valid = rows < len(arr)
            if np.any(valid):
                aligned[valid] = arr[rows[valid]].astype(np.float64)
            out[int(det_idx)] = aligned
        return out

    def _build_summary_tables(
        self,
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]],
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
            in_scan = self._get_scan_index(parsed_by_label, "input", sensor_id)
            out_scan = self._get_scan_index(parsed_by_label, "output", sensor_id)
            common = (
                np.intersect1d(in_scan, out_scan)
                if len(in_scan) and len(out_scan)
                else np.array([])
            )
            pct = self._pct(len(common), max(len(in_scan), len(out_scan)))
            in_sigs = len(
                self._get_all_signal_names(parsed_by_label, "input", sensor_id)
            )
            out_sigs = len(
                self._get_all_signal_names(parsed_by_label, "output", sensor_id)
            )
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

    def _payload(
        self,
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]],
        label: str,
        sensor_id: str,
    ) -> Dict[str, Any]:
        return parsed_by_label.get(label, {}).get(sensor_id, {})

    def _get_scan_index(
        self,
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]],
        label: str,
        sensor_id: str,
    ) -> np.ndarray:
        payload = self._payload(parsed_by_label, label, sensor_id)
        scan = payload.get("scan_index")
        return scan if isinstance(scan, np.ndarray) else np.array([], dtype=np.int64)

    def _get_header_signals(
        self,
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]],
        label: str,
        sensor_id: str,
    ) -> Dict[str, np.ndarray]:
        payload = self._payload(parsed_by_label, label, sensor_id)
        header = payload.get("header")
        return header if isinstance(header, dict) else {}

    def _get_all_signal_names(
        self,
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]],
        label: str,
        sensor_id: str,
    ) -> List[str]:
        payload = self._payload(parsed_by_label, label, sensor_id)
        detection = payload.get("detection")
        return sorted(detection.keys()) if isinstance(detection, dict) else []

    def _get_detection_signal(
        self,
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]],
        label: str,
        sensor_id: str,
        signal_name: str,
    ) -> Dict[int, np.ndarray]:
        payload = self._payload(parsed_by_label, label, sensor_id)
        detection = payload.get("detection")
        if not isinstance(detection, dict):
            return {}
        signal = detection.get(signal_name)
        return signal if isinstance(signal, dict) else {}

    def _get_detection_2d(
        self,
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]],
        label: str,
        sensor_id: str,
        signal_name: str,
    ) -> Tuple[np.ndarray, np.ndarray]:
        payload = self._payload(parsed_by_label, label, sensor_id)
        scan_index = self._get_scan_index(parsed_by_label, label, sensor_id)
        if len(scan_index) == 0:
            return np.array([], dtype=np.int64), np.empty((0, 0), dtype=np.float64)

        det_map = self._get_detection_signal(
            parsed_by_label, label, sensor_id, signal_name
        )
        if not det_map:
            return scan_index, np.empty((len(scan_index), 0), dtype=np.float64)

        valid = payload.get("header", {}).get("HED_NUM_OF_VALID_DETECTIONS")
        max_det = max(det_map.keys(), default=0)
        if not isinstance(valid, np.ndarray) or len(valid) < len(scan_index):
            valid = np.full(len(scan_index), max_det, dtype=np.int64)
        else:
            valid = np.clip(np.rint(valid[: len(scan_index)]).astype(np.int64), 0, None)

        rows: List[np.ndarray] = []
        for row in range(len(scan_index)):
            n_valid = max(0, min(int(valid[row]), max_det))
            vals = [
                float(det_map[det_idx][row])
                for det_idx in range(1, n_valid + 1)
                if isinstance(det_map.get(det_idx), np.ndarray)
                and row < len(det_map[det_idx])
            ]
            rows.append(np.asarray(vals, dtype=np.float64))

        width = max((len(r) for r in rows), default=0)
        if width == 0:
            return scan_index, np.empty((len(scan_index), 0), dtype=np.float64)

        out = np.full((len(rows), width), np.nan, dtype=np.float64)
        for idx, row_vals in enumerate(rows):
            if len(row_vals):
                out[idx, : len(row_vals)] = row_vals
        return scan_index, out

    def _aligned_common_rows(
        self, in_scan: np.ndarray, out_scan: np.ndarray
    ) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
        """Return exact scan-id aligned pairs (input row, output row).

        - Pairs are created only when scan IDs are identical.
        - If duplicates exist, they are paired one-by-one in encounter order.
        - Scan IDs present only on one side are skipped.
        """
        if len(in_scan) == 0 or len(out_scan) == 0:
            empty = np.array([], dtype=np.int64)
            return empty, empty, empty

        out_positions: Dict[int, List[int]] = {}
        for out_idx, scan in enumerate(out_scan):
            key = int(scan)
            out_positions.setdefault(key, []).append(out_idx)

        common_vals: List[int] = []
        in_rows: List[int] = []
        out_rows: List[int] = []

        for in_idx, scan in enumerate(in_scan):
            key = int(scan)
            pos_list = out_positions.get(key)
            if not pos_list:
                continue
            out_idx = pos_list.pop(0)
            common_vals.append(key)
            in_rows.append(in_idx)
            out_rows.append(out_idx)

        return (
            np.asarray(common_vals, dtype=np.int64),
            np.asarray(in_rows, dtype=np.int64),
            np.asarray(out_rows, dtype=np.int64),
        )
