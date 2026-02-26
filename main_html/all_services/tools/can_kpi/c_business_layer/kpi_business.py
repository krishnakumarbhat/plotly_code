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

    TIME_MATCH_TOL_NS = 2_000_000

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
        Compare input vs output detections per aligned scan.

                For each aligned scan pair:
                - Input/output rows are aligned by sensor timestamp.
                - Detections are paired greedily (nearest multi-signal distance),
                    so reordered detection indices do not break matching.
                - A detection counts as "matched" when **all four** parameter diffs
                    (range, range_velocity, azimuth, elevation) are within tolerance.
                - Values are rounded to 2 decimal places before comparison.
                - Only detections 1..N are considered, where N comes from the per-scan
                    header signal `HED_NUM_OF_VALID_DETECTIONS`.
                - Percentage denominator uses comparable pair capacity:
                    ``min(valid_input_detections, valid_output_detections)``.

        Returns
        -------
        common_scan : ndarray
        overall_pct : ndarray (float16)
        per_param_pct : dict {signal_name: ndarray (float16)}
        """
        in_scan = self._get_scan_index(parsed_by_label, "input", sensor_id)
        out_scan = self._get_scan_index(parsed_by_label, "output", sensor_id)
        in_time_ns = self._get_time_ns(parsed_by_label, "input", sensor_id)
        out_time_ns = self._get_time_ns(parsed_by_label, "output", sensor_id)
        common, in_rows, out_rows = self._aligned_common_rows(
            in_scan, out_scan, in_time_ns, out_time_ns
        )
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
        scan_mismatch_count = int(np.sum(in_scan_common != out_scan_common))
        if scan_mismatch_count > 0:
            logger.info(
                f"{sensor_id}: timestamp-aligned pairs={n}, scan-id mismatches={scan_mismatch_count}"
            )

        overall_pct = np.zeros(n, dtype=np.float16)
        per_param_pct: Dict[str, np.ndarray] = {
            sig: np.zeros(n, dtype=np.float16) for sig in self.MATCH_SIGNALS
        }

        for i in range(n):
            n_det = int(denom_cnt[i])
            if n_det <= 0:
                continue

            direct_all, direct_param, direct_den = self._evaluate_direct_pairing(
                in_sig, out_sig, i, n_det
            )
            in_candidates = self._collect_scan_detections(in_sig, i, n_det)
            out_candidates = self._collect_scan_detections(out_sig, i, n_det)
            if direct_den <= 0 and (not in_candidates or not out_candidates):
                continue

            greedy_all, greedy_param, greedy_den = self._evaluate_greedy_pairing(
                in_candidates, out_candidates
            )

            direct_pct = (100.0 * direct_all / direct_den) if direct_den > 0 else -1.0
            greedy_pct = (100.0 * greedy_all / greedy_den) if greedy_den > 0 else -1.0

            if greedy_pct > direct_pct:
                matched_all = greedy_all
                matched_param = greedy_param
                denom = greedy_den
            else:
                matched_all = direct_all
                matched_param = direct_param
                denom = direct_den

            if denom <= 0:
                continue

            overall_pct[i] = np.float16(round(100.0 * matched_all / float(denom), 2))
            for sig in self.MATCH_SIGNALS:
                per_param_pct[sig][i] = np.float16(
                    round(100.0 * matched_param[sig] / float(denom), 2)
                )

        return common, overall_pct, per_param_pct

    def _collect_scan_detections(
        self,
        sig_map: Dict[str, Dict[int, np.ndarray]],
        row_idx: int,
        n_det: int,
    ) -> List[Dict[str, float]]:
        detections: List[Dict[str, float]] = []
        for det_idx in range(1, n_det + 1):
            values: Dict[str, float] = {}
            valid = True
            for sig in self.MATCH_SIGNALS:
                arr = sig_map.get(sig, {}).get(det_idx)
                if arr is None or row_idx >= len(arr):
                    valid = False
                    break
                value = float(arr[row_idx])
                if math.isnan(value):
                    valid = False
                    break
                values[sig] = value

            if not valid:
                continue
            if values.get("DET_RANGE", 0.0) == 0.0:
                continue
            detections.append(values)
        return detections

    def _evaluate_direct_pairing(
        self,
        in_sig: Dict[str, Dict[int, np.ndarray]],
        out_sig: Dict[str, Dict[int, np.ndarray]],
        row_idx: int,
        n_det: int,
    ) -> Tuple[int, Dict[str, int], int]:
        matched_all = 0
        matched_param = {sig: 0 for sig in self.MATCH_SIGNALS}
        denom = 0

        for det_idx in range(1, n_det + 1):
            rin_arr = in_sig.get("DET_RANGE", {}).get(det_idx)
            rout_arr = out_sig.get("DET_RANGE", {}).get(det_idx)
            if rin_arr is None or rout_arr is None:
                continue
            if row_idx >= len(rin_arr) or row_idx >= len(rout_arr):
                continue

            rin = float(rin_arr[row_idx])
            rout = float(rout_arr[row_idx])
            if math.isnan(rin) or math.isnan(rout) or rin == 0.0 or rout == 0.0:
                continue

            denom += 1
            det_all_ok = True
            for sig in self.MATCH_SIGNALS:
                ain_arr = in_sig.get(sig, {}).get(det_idx)
                aout_arr = out_sig.get(sig, {}).get(det_idx)
                if ain_arr is None or aout_arr is None:
                    det_all_ok = False
                    continue
                if row_idx >= len(ain_arr) or row_idx >= len(aout_arr):
                    det_all_ok = False
                    continue

                ain = float(ain_arr[row_idx])
                aout = float(aout_arr[row_idx])
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

        return matched_all, matched_param, denom

    def _evaluate_greedy_pairing(
        self,
        in_candidates: List[Dict[str, float]],
        out_candidates: List[Dict[str, float]],
    ) -> Tuple[int, Dict[str, int], int]:
        if not in_candidates or not out_candidates:
            return 0, {sig: 0 for sig in self.MATCH_SIGNALS}, 0

        used_out = np.zeros(len(out_candidates), dtype=bool)
        matched_all = 0
        matched_param = {sig: 0 for sig in self.MATCH_SIGNALS}

        for in_det in in_candidates:
            best_out_idx = self._pick_best_output_detection(
                in_det, out_candidates, used_out
            )
            if best_out_idx < 0:
                continue
            used_out[best_out_idx] = True

            out_det = out_candidates[best_out_idx]
            det_all_ok = True
            for sig in self.MATCH_SIGNALS:
                ain = round(float(in_det[sig]), 2)
                aout = round(float(out_det[sig]), 2)
                if abs(ain - aout) <= float(self.MATCH_TOLERANCES[sig]):
                    matched_param[sig] += 1
                else:
                    det_all_ok = False

            if det_all_ok:
                matched_all += 1

        denom = min(len(in_candidates), len(out_candidates))
        return matched_all, matched_param, denom

    def _pick_best_output_detection(
        self,
        in_det: Dict[str, float],
        out_candidates: List[Dict[str, float]],
        used_out: np.ndarray,
    ) -> int:
        best_idx = -1
        best_cost = float("inf")
        for idx, out_det in enumerate(out_candidates):
            if used_out[idx]:
                continue
            cost = 0.0
            for sig in self.MATCH_SIGNALS:
                cost += abs(
                    round(float(in_det[sig]), 2) - round(float(out_det[sig]), 2)
                )
            if cost < best_cost:
                best_cost = cost
                best_idx = idx
        return best_idx

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
            in_time_ns = self._get_time_ns(parsed_by_label, "input", sensor_id)
            out_time_ns = self._get_time_ns(parsed_by_label, "output", sensor_id)
            common, _, _ = self._aligned_common_rows(
                in_scan, out_scan, in_time_ns, out_time_ns
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
        self,
        in_scan: np.ndarray,
        out_scan: np.ndarray,
        in_time_ns: np.ndarray,
        out_time_ns: np.ndarray,
    ) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
        """Return aligned pairs using absolute timestamp with tolerance.

        - Pairing is done by monotonic two-pointer match on ``time_ns``.
        - Pairs are accepted when |in_time - out_time| <= TIME_MATCH_TOL_NS.
        - Missing/jumbled scan-index sequences are tolerated.
        """
        if len(in_scan) == 0 or len(out_scan) == 0:
            empty = np.array([], dtype=np.int64)
            return empty, empty, empty

        # If time arrays are unavailable, fall back to scan-index-only pairing.
        if len(in_time_ns) == 0 or len(out_time_ns) == 0:
            return self._aligned_common_rows_scan_only(in_scan, out_scan)

        n_in = min(len(in_scan), len(in_time_ns))
        n_out = min(len(out_scan), len(out_time_ns))
        if n_in == 0 or n_out == 0:
            empty = np.array([], dtype=np.int64)
            return empty, empty, empty

        in_scan = in_scan[:n_in]
        out_scan = out_scan[:n_out]
        in_time_ns = in_time_ns[:n_in]
        out_time_ns = out_time_ns[:n_out]

        common_vals: List[int] = []
        in_rows: List[int] = []
        out_rows: List[int] = []

        i = 0
        j = 0
        tol = int(self.TIME_MATCH_TOL_NS)
        while i < n_in and j < n_out:
            dt = int(in_time_ns[i]) - int(out_time_ns[j])
            if abs(dt) <= tol:
                common_vals.append(int(in_scan[i]))
                in_rows.append(i)
                out_rows.append(j)
                i += 1
                j += 1
            elif dt < 0:
                i += 1
            else:
                j += 1

        return (
            np.asarray(common_vals, dtype=np.int64),
            np.asarray(in_rows, dtype=np.int64),
            np.asarray(out_rows, dtype=np.int64),
        )

    def _aligned_common_rows_scan_only(
        self, in_scan: np.ndarray, out_scan: np.ndarray
    ) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
        """Fallback alignment when timestamp arrays are not available."""
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

    def _get_time_ns(
        self,
        parsed_by_label: Dict[str, Dict[str, Dict[str, Any]]],
        label: str,
        sensor_id: str,
    ) -> np.ndarray:
        payload = self._payload(parsed_by_label, label, sensor_id)
        t = payload.get("time_ns")
        return t if isinstance(t, np.ndarray) else np.array([], dtype=np.int64)
