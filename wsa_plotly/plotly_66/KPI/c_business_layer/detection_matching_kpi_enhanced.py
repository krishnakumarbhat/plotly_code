import logging
from dataclasses import dataclass
from typing import Dict, Tuple, List

import numpy as np

from KPI.b_data_storage.kpi_data_model_storage import KPI_DataModelStorage
from KPI.b_data_storage.kpi_config_storage import KPI_VALIDATION_RULES

logger = logging.getLogger(__name__)


@dataclass
class Thresholds:
    ran: float
    vel: float
    theta: float
    phi: float
    radar_cycle_s: float
    max_cdc_records: int
    range_sat_front: float
    range_sat_corner: float
    max_af_front: int
    max_af_corner: int
    max_rdd: int


class DetectionMappingKPI:
    """Feature-parity port of the CSV KPI script for HDF-backed storage."""

    def __init__(self, data: KPI_DataModelStorage, sensor_id: str):
        self.data = data
        self.sensor_id = sensor_id
        cfg = KPI_VALIDATION_RULES["detection_thresholds"]
        self.thresholds = Thresholds(
            ran=cfg["range_threshold"],
            vel=cfg["velocity_threshold"],
            theta=cfg["theta_threshold"],
            phi=cfg["phi_threshold"],
            radar_cycle_s=cfg["radar_cycle_s"],
            max_cdc_records=cfg["max_cdc_records"],
            range_sat_front=cfg["range_saturation_threshold_front"],
            range_sat_corner=cfg["range_saturation_threshold_corner"],
            max_af_front=cfg["max_num_af_dets_front"],
            max_af_corner=cfg["max_num_af_dets_corner"],
            max_rdd=cfg["max_num_rdd_dets"],
        )
        if "FRONT" in sensor_id.upper() or "_FC_" in sensor_id.upper():
            self.max_af = self.thresholds.max_af_front
            self.range_sat_threshold = self.thresholds.range_sat_front
        else:
            self.max_af = self.thresholds.max_af_corner
            self.range_sat_threshold = self.thresholds.range_sat_corner

        self.results: Dict[str, Dict[str, float]] = {}

    # ------------------------------------------------------------------ helpers
    @staticmethod
    def _extract(arr: np.ndarray, width: int = None) -> Tuple[np.ndarray, np.ndarray]:
        if arr.size == 0:
            scan = np.array([], dtype=int)
            values = np.empty((0, width or 0))
            return scan, values
        if arr.ndim == 1:
            scan = np.array([int(arr[0])])
            values = arr[1:].reshape(1, -1)
        else:
            scan = arr[:, 0].astype(int)
            values = arr[:, 1:]
        if width is not None and values.shape[1] != width:
            values = DetectionMappingKPI._pad(values, width)
        return scan, values

    @staticmethod
    def _pad(values: np.ndarray, width: int) -> np.ndarray:
        current = values.shape[1]
        if current >= width:
            return values[:, :width]
        pad = width - current
        return np.pad(values, ((0, 0), (0, pad)), constant_values=np.nan)

    @staticmethod
    def _index(scan: np.ndarray) -> Dict[int, int]:
        return {int(si): idx for idx, si in enumerate(scan)}

    # ------------------------------------------------------------------ main flow
    def run(self) -> bool:
        try:
            det_stream = self.data.get("DETECTION_STREAM", {})
            if not det_stream:
                logger.error("DETECTION_STREAM missing in KPI data")
                return False

            veh_det, sim_det = self._load_det_stream(det_stream)
            if veh_det is None:
                return False

            rdd_stream = self.data.get("RDD_STREAM", {})
            veh_rdd, sim_rdd = self._load_rdd_stream(rdd_stream)

            rdd_kpis = self._compute_rdd_kpis(veh_rdd, sim_rdd)
            det_kpis = self._compute_det_kpis(veh_det, sim_det, veh_rdd, sim_rdd)
            cdc_kpis = self._compute_cdc_metrics()
            vse_kpis = self._compute_vse_metrics(det_kpis)

            self.results = {
                "rdd": rdd_kpis,
                "det": det_kpis,
                "cdc": cdc_kpis,
                "vse": vse_kpis,
            }
            return True
        except Exception as exc:
            logger.exception("Detection KPI processing failed: %s", exc)
            return False

    # ------------------------------------------------------------------ data loading
    def _load_det_stream(self, stream: Dict) -> Tuple[Dict[str, np.ndarray], Dict[str, np.ndarray]]:
        params = [
            "num_af_det",
            "rdd_idx",
            "ran",
            "vel",
            "theta",
            "phi",
            "f_single_target",
            "f_superres_target",
            "f_bistatic",
        ]
        veh: Dict[str, np.ndarray] = {}
        sim: Dict[str, np.ndarray] = {}
        for name in params:
            veh_arr, veh_status = KPI_DataModelStorage.get_value(stream["input"], name)
            sim_arr, sim_status = KPI_DataModelStorage.get_value(stream["output"], name)
            if veh_status != "success" or sim_status != "success":
                logger.error("DET signal %s failed (veh=%s, sim=%s)", name, veh_status, sim_status)
                return None, None
            veh[name] = veh_arr
            sim[name] = sim_arr
        return veh, sim

    def _load_rdd_stream(self, stream: Dict) -> Tuple[Dict[str, np.ndarray], Dict[str, np.ndarray]]:
        if not stream:
            return {}, {}
        params = ["rdd1_num_detect", "rdd1_rindx", "rdd1_dindx", "rdd2_range", "rdd2_range_rate"]
        veh: Dict[str, np.ndarray] = {}
        sim: Dict[str, np.ndarray] = {}
        for name in params:
            veh_arr, veh_status = KPI_DataModelStorage.get_value(stream["input"], name)
            sim_arr, sim_status = KPI_DataModelStorage.get_value(stream["output"], name)
            if veh_status != "success" or sim_status != "success":
                logger.warning("RDD signal %s failed (veh=%s, sim=%s) — KPIs will ignore it", name, veh_status, sim_status)
                veh[name] = np.empty((0, 0))
                sim[name] = np.empty((0, 0))
            else:
                veh[name] = veh_arr
                sim[name] = sim_arr
        return veh, sim

    # ------------------------------------------------------------------ RDD KPIs
    def _compute_rdd_kpis(self, veh: Dict[str, np.ndarray], sim: Dict[str, np.ndarray]) -> Dict[str, float]:
        if not veh or not sim:
            return {}

        veh_si, veh_counts = self._extract(veh["rdd1_num_detect"], 1)
        sim_si, sim_counts = self._extract(sim["rdd1_num_detect"], 1)
        veh_map = self._index(veh_si)
        sim_map = self._index(sim_si)
        common = sorted(set(veh_map) & set(sim_map))
        if not common:
            return {}

        veh_rindx = self._extract(veh["rdd1_rindx"], self.thresholds.max_rdd)[1]
        veh_dindx = self._extract(veh["rdd1_dindx"], self.thresholds.max_rdd)[1]
        veh_range = self._extract(veh["rdd2_range"], self.thresholds.max_rdd)[1]
        veh_rate = self._extract(veh["rdd2_range_rate"], self.thresholds.max_rdd)[1]
        sim_rindx = self._extract(sim["rdd1_rindx"], self.thresholds.max_rdd)[1]
        sim_dindx = self._extract(sim["rdd1_dindx"], self.thresholds.max_rdd)[1]
        sim_range = self._extract(sim["rdd2_range"], self.thresholds.max_rdd)[1]
        sim_rate = self._extract(sim["rdd2_range_rate"], self.thresholds.max_rdd)[1]

        same_num_count = 0
        pair_match_scans = 0
        value_match_scans = 0
        total_scans = len(common)

        for scan in common:
            vi = veh_map[scan]
            si = sim_map[scan]
            veh_count = int(np.nan_to_num(veh_counts[vi, 0], nan=0))
            sim_count = int(np.nan_to_num(sim_counts[si, 0], nan=0))
            if veh_count == sim_count and veh_count > 0:
                same_num_count += 1

            veh_pairs = {(int(veh_rindx[vi, j]), int(veh_dindx[vi, j])) for j in range(min(veh_count, self.thresholds.max_rdd)) if not np.isnan(veh_rindx[vi, j])}
            sim_pairs = {(int(sim_rindx[si, j]), int(sim_dindx[si, j])) for j in range(min(sim_count, self.thresholds.max_rdd)) if not np.isnan(sim_rindx[si, j])}
            if not veh_pairs or not sim_pairs:
                continue
            matches = veh_pairs & sim_pairs
            if matches:
                pair_match_scans += 1
                good_pairs = 0
                for rinx, dinx in matches:
                    v_idx = self._find_index(veh_rindx[vi], rinx)
                    s_idx = self._find_index(sim_rindx[si], rinx)
                    if v_idx is None or s_idx is None:
                        continue
                    range_diff = abs(veh_range[vi, v_idx] - sim_range[si, s_idx])
                    rate_diff = abs(veh_rate[vi, v_idx] - sim_rate[si, s_idx])
                    if range_diff <= self.thresholds.ran and rate_diff <= self.thresholds.vel:
                        good_pairs += 1
                if good_pairs == len(matches) and len(matches) > 0:
                    value_match_scans += 1

        return {
            "scans_compared": total_scans,
            "same_num_dets_scans": same_num_count,
            "same_num_dets_pct": round(same_num_count / total_scans * 100, 2) if total_scans else 0.0,
            "pair_match_scans": pair_match_scans,
            "pair_match_pct": round(pair_match_scans / total_scans * 100, 2) if total_scans else 0.0,
            "range_rate_match_scans": value_match_scans,
            "range_rate_match_pct": round(value_match_scans / total_scans * 100, 2) if total_scans else 0.0,
        }

    @staticmethod
    def _find_index(values: np.ndarray, needle: int) -> int:
        for idx, val in enumerate(values):
            if int(val) == needle:
                return idx
        return None

    # ------------------------------------------------------------------ Detection KPIs
    def _compute_det_kpis(
        self,
        veh_det: Dict[str, np.ndarray],
        sim_det: Dict[str, np.ndarray],
        veh_rdd: Dict[str, np.ndarray],
        sim_rdd: Dict[str, np.ndarray],
    ) -> Dict[str, float]:
        veh_si, veh_counts = self._extract(veh_det["num_af_det"], 1)
        sim_si, sim_counts = self._extract(sim_det["num_af_det"], 1)
        veh_map = self._index(veh_si)
        sim_map = self._index(sim_si)
        common = sorted(set(veh_map) & set(sim_map))
        if not common:
            return {}

        veh_rdd_idx = self._extract(veh_det["rdd_idx"], self.max_af)[1]
        veh_ran = self._extract(veh_det["ran"], self.max_af)[1]
        veh_vel = self._extract(veh_det["vel"], self.max_af)[1]
        veh_theta = self._extract(veh_det["theta"], self.max_af)[1]
        veh_phi = self._extract(veh_det["phi"], self.max_af)[1]

        sim_rdd_idx = self._extract(sim_det["rdd_idx"], self.max_af)[1]
        sim_ran = self._extract(sim_det["ran"], self.max_af)[1]
        sim_vel = self._extract(sim_det["vel"], self.max_af)[1]
        sim_theta = self._extract(sim_det["theta"], self.max_af)[1]
        sim_phi = self._extract(sim_det["phi"], self.max_af)[1]

        same_num_scans = 0
        subset_match_scans = 0
        full_match_scans = 0
        total_subset_matches = 0
        total_full_matches = 0
        total_veh_dets = 0

        for scan in common:
            vi = veh_map[scan]
            si = sim_map[scan]
            veh_count = int(np.nan_to_num(veh_counts[vi, 0], nan=0))
            sim_count = int(np.nan_to_num(sim_counts[si, 0], nan=0))
            total_veh_dets += veh_count
            if veh_count == sim_count:
                same_num_scans += 1

            if veh_count == 0 or sim_count == 0:
                continue

            sim_lookup: Dict[int, List[int]] = {}
            for j in range(min(sim_count, self.max_af)):
                key = int(np.nan_to_num(sim_rdd_idx[si, j], nan=-1))
                if key < 0:
                    continue
                sim_lookup.setdefault(key, []).append(j)

            subset_matches = 0
            full_matches = 0

            for j in range(min(veh_count, self.max_af)):
                key = int(np.nan_to_num(veh_rdd_idx[vi, j], nan=-1))
                if key not in sim_lookup:
                    continue
                k = sim_lookup[key].pop(0)
                if not sim_lookup[key]:
                    sim_lookup.pop(key)

                ran_diff = abs(veh_ran[vi, j] - sim_ran[si, k])
                vel_diff = abs(veh_vel[vi, j] - sim_vel[si, k])
                theta_diff = abs(veh_theta[vi, j] - sim_theta[si, k])
                phi_diff = abs(veh_phi[vi, j] - sim_phi[si, k])

                if ran_diff <= self.thresholds.ran and vel_diff <= self.thresholds.vel:
                    subset_matches += 1
                    if theta_diff <= self.thresholds.theta and phi_diff <= self.thresholds.phi:
                        full_matches += 1

            total_subset_matches += subset_matches
            total_full_matches += full_matches

            if subset_matches == veh_count:
                subset_match_scans += 1
            if full_matches == veh_count:
                full_match_scans += 1

        return {
            "num_scans": len(common),
            "same_num_scans": same_num_scans,
            "same_num_pct": round(same_num_scans / len(common) * 100, 2),
            "subset_match_scans": subset_match_scans,
            "subset_match_pct": round(subset_match_scans / len(common) * 100, 2) if common else 0.0,
            "full_match_scans": full_match_scans,
            "full_match_pct": round(full_match_scans / len(common) * 100, 2) if common else 0.0,
            "total_dets_veh": total_veh_dets,
            "matched_subset_dets": total_subset_matches,
            "matched_full_dets": total_full_matches,
            "accuracy_subset": round(total_subset_matches / total_veh_dets * 100, 2) if total_veh_dets else 0.0,
            "accuracy_full": round(total_full_matches / total_veh_dets * 100, 2) if total_veh_dets else 0.0,
        }

    # ------------------------------------------------------------------ CDC / VSE metrics
    def _compute_cdc_metrics(self) -> Dict[str, float]:
        stream = self.data.get("CDC_STREAM", {})
        if not stream:
            return {}
        num_arr, status = KPI_DataModelStorage.get_value(stream["input"], "num_cdc_records")
        if status != "success" or num_arr.size == 0:
            return {}
        scans, counts = self._extract(num_arr, 1)
        saturated = counts[:, 0] >= self.thresholds.max_cdc_records
        return {
            "num_scans": len(scans),
            "saturated_scans": int(np.sum(saturated)),
            "saturated_pct": round(np.mean(saturated) * 100, 2) if len(scans) else 0.0,
        }

    def _compute_vse_metrics(self, det_kpis: Dict[str, float]) -> Dict[str, float]:
        stream = self.data.get("VSE_STREAM", {})
        if not stream:
            return {}
        veh_vse, status = KPI_DataModelStorage.get_value(stream["input"], "veh_speed")
        sim_vse, status2 = KPI_DataModelStorage.get_value(stream["output"], "veh_speed")
        if status != "success" or status2 != "success" or veh_vse.size == 0 or sim_vse.size == 0:
            return {}
        veh_scan, veh_speed = self._extract(veh_vse, 1)
        sim_scan, sim_speed = self._extract(sim_vse, 1)
        veh_dist = float(np.nansum(veh_speed[:, 0]) * self.thresholds.radar_cycle_s)
        sim_dist = float(np.nansum(sim_speed[:, 0]) * self.thresholds.radar_cycle_s)
        scan_yield = 0.0
        if det_kpis.get("num_scans"):
            scan_yield = round(det_kpis.get("same_num_scans", 0) / det_kpis["num_scans"] * 100, 2)
        mileage_yield = round(sim_dist / veh_dist * 100, 2) if veh_dist else 0.0
        return {
            "veh_distance_m": veh_dist,
            "sim_distance_m": sim_dist,
            "scan_yield_pct": scan_yield,
            "mileage_yield_pct": mileage_yield,
        }
