"""Business orchestration — uses storage objects for KPI matching and plotting."""

import logging
import math
from itertools import product
from collections import OrderedDict
from typing import Any, Dict, List, Optional

import numpy as np

from a_persistence_layer.hdf_parser import KpiHdfParser
from a_persistence_layer.json_parser import KpiJsonParser
from b_data_storage.can_kpi_data_model_storage import KPI_DataModelStorage
from d_presentation_layer.kpi_html_gen import KpiHtmlGen

logger = logging.getLogger(__name__)


class KpiBusiness:
    MATCH_SIGNALS = ["DET_RANGE", "DET_RANGE_VELOCITY", "DET_AZIMUTH", "DET_ELEVATION"]
    MATCH_EPSILON = 0.01
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
    ):
        self._json = json_parser or KpiJsonParser()
        required_signals = sorted(set(self.MATCH_SIGNALS))
        self._hdf = hdf_parser or KpiHdfParser(
            required_detection_signals=required_signals
        )
        self._html = html or KpiHtmlGen()
        self._offsets_4d = sorted(
            list(product([-1, 0, 1], repeat=4)),
            key=lambda o: abs(o[0]) + abs(o[1]) + abs(o[2]) + abs(o[3]),
        )
        self._offsets_1d = [(-1,), (0,), (1,)]

    def run(self, kpi_json_path: str) -> Dict[str, Any]:
        cfg = self._json.parse(kpi_json_path)
        in_path = (cfg.get("INPUT_HDF", []) or [None])[0]
        out_path = (cfg.get("OUTPUT_HDF", []) or [None])[0]
        return self.run_pair(in_path, out_path, title="PCAN Detection KPI")

    def run_pair(
        self,
        input_hdf: Optional[str],
        output_hdf: Optional[str],
        title: str = "PCAN Detection KPI",
    ) -> Dict[str, Any]:
        if not input_hdf and not output_hdf:
            raise ValueError("At least one of input_hdf/output_hdf must be provided")

        in_parsed = self._hdf.parse_file(input_hdf) if input_hdf else {}
        out_parsed = self._hdf.parse_file(output_hdf) if output_hdf else {}
        in_stores = self._hdf.extract_storages(in_parsed)
        out_stores = self._hdf.extract_storages(out_parsed)

        all_sensors = sorted(
            set(in_stores.keys()) | set(out_stores.keys()),
            key=lambda s: self.SENSOR_ORDER.index(s) if s in self.SENSOR_ORDER else 99,
        )

        match_results: Dict[str, Dict[str, Any]] = {
            sensor_id: self._compute_match_pct(
                in_stores.get(sensor_id), out_stores.get(sensor_id), sensor_id
            )
            for sensor_id in all_sensors
        }

        summary_html = self._build_summary_tables(in_stores, out_stores, all_sensors)

        sensor_tabs: OrderedDict[str, str] = OrderedDict()
        for sensor_id in all_sensors:
            label = self.FRIENDLY.get(sensor_id, sensor_id)
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
        radar_plot_data: Dict[str, tuple] = {}
        for sensor_id in all_sensors:
            result = match_results.get(sensor_id, {})
            scan = result.get("scan", np.array([], dtype=np.int64))
            overall = result.get("overall", np.array([], dtype=np.float16))
            precision = result.get("precision", np.array([], dtype=np.float16))
            recall = result.get("recall", np.array([], dtype=np.float16))
            f1 = result.get("f1", np.array([], dtype=np.float16))
            accuracy = result.get("accuracy", np.array([], dtype=np.float16))

            radar_name = self.FRIENDLY.get(sensor_id, sensor_id)
            radar_plot_data[radar_name] = (scan, overall)
            kpi_rows.append(
                [
                    radar_name,
                    str(len(scan)),
                    f"{self._avg(overall):.2f}",
                    f"{self._avg(precision):.2f}",
                    f"{self._avg(recall):.2f}",
                    f"{self._avg(f1):.2f}",
                    f"{self._avg(accuracy):.2f}",
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

        signals: Dict[str, List[str]] = {}
        for sid, store in in_stores.items():
            signals[f"input/{sid}"] = store.get_detection_signal_names()
        for sid, store in out_stores.items():
            signals[f"output/{sid}"] = store.get_detection_signal_names()

        return {
            "html": html,
            "signals": signals,
            "storages": {"input": in_stores, "output": out_stores},
        }

    def _compute_match_pct(
        self,
        in_store: Optional[KPI_DataModelStorage],
        out_store: Optional[KPI_DataModelStorage],
        sensor_id: str,
    ) -> Dict[str, Any]:
        empty = np.array([], dtype=np.float16)
        empty_params = {s: empty for s in self.MATCH_SIGNALS}
        if in_store is None or out_store is None:
            return {
                "scan": np.array([], dtype=np.int64),
                "overall": empty,
                "precision": empty,
                "recall": empty,
                "f1": empty,
                "accuracy": empty,
                "per_signal": empty_params,
            }

        common_scan, in_rows, out_rows = self._hdf.align_storage_rows(
            in_store, out_store, time_tolerance_ns=self.TIME_MATCH_TOL_NS
        )
        n = len(common_scan)
        if n == 0:
            return {
                "scan": common_scan,
                "overall": empty,
                "precision": empty,
                "recall": empty,
                "f1": empty,
                "accuracy": empty,
                "per_signal": empty_params,
            }

        in_cnt_all = in_store.get_valid_detection_counts()
        out_cnt_all = out_store.get_valid_detection_counts()
        in_cnt = self._safe_take(in_cnt_all, in_rows)
        out_cnt = self._safe_take(out_cnt_all, out_rows)
        denom_cnt = np.minimum(in_cnt, out_cnt).astype(np.int32)

        if np.any(in_cnt != out_cnt):
            logger.warning(
                f"{sensor_id}: HED_NUM_OF_VALID_DETECTIONS differs between input/output "
                f"for {int(np.sum(in_cnt != out_cnt))} scans; using min(in,out)"
            )

        overall = np.zeros(n, dtype=np.float16)
        precision = np.zeros(n, dtype=np.float16)
        recall = np.zeros(n, dtype=np.float16)
        f1 = np.zeros(n, dtype=np.float16)
        accuracy = np.zeros(n, dtype=np.float16)
        per_param = {sig: np.zeros(n, dtype=np.float16) for sig in self.MATCH_SIGNALS}

        for idx in range(n):
            n_det = int(denom_cnt[idx])
            if n_det <= 0:
                continue

            in_candidates = in_store.get_scan_detections(
                self.MATCH_SIGNALS, int(in_rows[idx]), n_det
            )
            out_candidates = out_store.get_scan_detections(
                self.MATCH_SIGNALS, int(out_rows[idx]), n_det
            )
            if not in_candidates or not out_candidates:
                continue

            tp_all = self._match_detections_hashmap(in_candidates, out_candidates)
            in_n = len(in_candidates)
            out_n = len(out_candidates)
            denom = min(in_n, out_n)
            if denom <= 0:
                continue

            tp_sig: Dict[str, int] = {}
            for sig in self.MATCH_SIGNALS:
                in_vals = [row[sig] for row in in_candidates]
                out_vals = [row[sig] for row in out_candidates]
                tp_sig[sig] = self._match_1d_hashmap(in_vals, out_vals)

            fp = max(0, out_n - tp_all)
            fn = max(0, in_n - tp_all)

            prec = tp_all / (tp_all + fp) if (tp_all + fp) > 0 else 0.0
            rec = tp_all / (tp_all + fn) if (tp_all + fn) > 0 else 0.0
            f1_val = (2.0 * prec * rec / (prec + rec)) if (prec + rec) > 0 else 0.0
            acc = tp_all / (tp_all + fp + fn) if (tp_all + fp + fn) > 0 else 0.0

            overall[idx] = np.float16(round(100.0 * (tp_all / float(denom)), 2))
            precision[idx] = np.float16(round(100.0 * prec, 2))
            recall[idx] = np.float16(round(100.0 * rec, 2))
            f1[idx] = np.float16(round(100.0 * f1_val, 2))
            accuracy[idx] = np.float16(round(100.0 * acc, 2))

            for sig in self.MATCH_SIGNALS:
                per_param[sig][idx] = np.float16(
                    round(100.0 * (tp_sig[sig] / float(denom)), 2)
                )

        return {
            "scan": common_scan,
            "overall": overall,
            "precision": precision,
            "recall": recall,
            "f1": f1,
            "accuracy": accuracy,
            "per_signal": per_param,
        }

    def _quantize(self, value: float) -> int:
        return int(round(float(value) / self.MATCH_EPSILON))

    def _match_1d_hashmap(self, in_vals: List[float], out_vals: List[float]) -> int:
        out_map: Dict[tuple[int], int] = {}
        for v in out_vals:
            k = (self._quantize(v),)
            out_map[k] = out_map.get(k, 0) + 1

        matches = 0
        for v in in_vals:
            base = self._quantize(v)
            used = False
            for off in self._offsets_1d:
                key = (base + off[0],)
                cnt = out_map.get(key, 0)
                if cnt > 0:
                    matches += 1
                    if cnt == 1:
                        del out_map[key]
                    else:
                        out_map[key] = cnt - 1
                    used = True
                    break
            if used:
                continue
        return matches

    def _match_detections_hashmap(
        self,
        in_candidates: List[Dict[str, float]],
        out_candidates: List[Dict[str, float]],
    ) -> int:
        out_map: Dict[tuple[int, int, int, int], int] = {}
        for row in out_candidates:
            key = (
                self._quantize(row["DET_RANGE"]),
                self._quantize(row["DET_RANGE_VELOCITY"]),
                self._quantize(row["DET_AZIMUTH"]),
                self._quantize(row["DET_ELEVATION"]),
            )
            out_map[key] = out_map.get(key, 0) + 1

        matches = 0
        for row in in_candidates:
            base = (
                self._quantize(row["DET_RANGE"]),
                self._quantize(row["DET_RANGE_VELOCITY"]),
                self._quantize(row["DET_AZIMUTH"]),
                self._quantize(row["DET_ELEVATION"]),
            )

            for off in self._offsets_4d:
                key = (
                    base[0] + off[0],
                    base[1] + off[1],
                    base[2] + off[2],
                    base[3] + off[3],
                )
                cnt = out_map.get(key, 0)
                if cnt > 0:
                    matches += 1
                    if cnt == 1:
                        del out_map[key]
                    else:
                        out_map[key] = cnt - 1
                    break
        return matches

    def _safe_take(self, arr: np.ndarray, rows: np.ndarray) -> np.ndarray:
        if not isinstance(arr, np.ndarray) or len(arr) == 0 or len(rows) == 0:
            return np.zeros(len(rows), dtype=np.int32)
        out = np.zeros(len(rows), dtype=np.int32)
        valid = rows < len(arr)
        if np.any(valid):
            out[valid] = np.clip(np.rint(arr[rows[valid]]).astype(np.int32), 0, None)
        return out

    def _build_summary_tables(
        self,
        in_stores: Dict[str, KPI_DataModelStorage],
        out_stores: Dict[str, KPI_DataModelStorage],
        all_sensors: List[str],
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
            in_store = in_stores.get(sensor_id)
            out_store = out_stores.get(sensor_id)

            in_scan = (
                in_store.get_scan_index() if in_store else np.array([], dtype=np.int64)
            )
            out_scan = (
                out_store.get_scan_index()
                if out_store
                else np.array([], dtype=np.int64)
            )
            if in_store is not None and out_store is not None:
                common, _, _ = self._hdf.align_storage_rows(
                    in_store, out_store, time_tolerance_ns=self.TIME_MATCH_TOL_NS
                )
            else:
                common = np.array([], dtype=np.int64)

            pct = self._pct(len(common), max(len(in_scan), len(out_scan)))
            in_sigs = len(in_store.get_detection_signal_names()) if in_store else 0
            out_sigs = len(out_store.get_detection_signal_names()) if out_store else 0

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

    def _avg(self, arr: np.ndarray) -> float:
        if not isinstance(arr, np.ndarray) or len(arr) == 0:
            return 0.0
        return float(np.nanmean(arr.astype(float)))

    def _pct(self, num: int, den: int) -> float:
        if den <= 0:
            return float("nan")
        return 100.0 * float(num) / float(den)
