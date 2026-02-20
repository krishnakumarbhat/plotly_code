"""
CAN KPI Data Storage — lightweight storage for parsed CAN-bus radar data.

This module provides ``CANKpiDataStore`` which holds:
  * Per-sensor scan indices
  * 2-D detection arrays  (n_scans × max_det_index)  per signal type
  * Alignment & header scalars per scan
  * Convenience accessors for the plotting / business layers
"""

from typing import Dict, List, Any, Optional, Tuple
import numpy as np
import logging

logger = logging.getLogger(__name__)


class CANKpiDataStore:
    """Centralised store for all sensor data parsed from input & output HDF files."""

    def __init__(self):
        # {hdf_label: {sensor_id: sensor_dict}}
        # sensor_dict = result of parse_hdf_file per sensor
        self._stores: Dict[str, Dict[str, Any]] = {}
        self._det2d_cache: Dict[Tuple[str, str, str], Tuple[np.ndarray, np.ndarray]] = {}

    # ── Loading API ─────────────────────────────────────────────────────
    def add_parsed_data(self, label: str, parsed: Dict[str, Any]) -> None:
        """
        Add already-parsed HDF data (from ``kpi_hdf_parser.parse_hdf_file``).

        Parameters
        ----------
        label : str
            "input" or "output" (or any user-supplied label).
        parsed : dict
            ``{sensor_id: {friendly_name, scan_index, detection_signals, ...}}``.
        """
        self._stores[label] = parsed
        self._det2d_cache = {}
        for sid, sd in parsed.items():
            logger.info(
                f"[{label}] Added sensor {sid} ({sd.get('friendly_name')}) "
                f"— {sd.get('n_scans', 0)} scans"
            )

    # ── Query API ───────────────────────────────────────────────────────
    def labels(self) -> List[str]:
        return list(self._stores.keys())

    def sensor_ids(self, label: str) -> List[str]:
        return list(self._stores.get(label, {}).keys())

    def get_sensor(self, label: str, sensor_id: str) -> Optional[Dict[str, Any]]:
        return self._stores.get(label, {}).get(sensor_id)

    def get_scan_index(self, label: str, sensor_id: str) -> np.ndarray:
        sd = self.get_sensor(label, sensor_id)
        if sd is None:
            return np.array([])
        return sd.get("scan_index", np.array([]))

    def get_detection_signal(
        self, label: str, sensor_id: str, signal_name: str
    ) -> Optional[Dict[int, np.ndarray]]:
        """Return {det_idx: ndarray(n_scans,)} for one signal type, or None."""
        sd = self.get_sensor(label, sensor_id)
        if sd is None:
            return None
        return sd.get("detection_signals", {}).get(signal_name)

    def get_detection_2d(
        self, label: str, sensor_id: str, signal_name: str
    ) -> Tuple[np.ndarray, np.ndarray]:
        """Return (scan_index, 2d_array) where 2d_array is (n_scans, max_det).

        NaN for missing detection slots.
        """
        cache_key = (label, sensor_id, signal_name)
        cached = self._det2d_cache.get(cache_key)
        if cached is not None:
            return cached

        sd = self.get_sensor(label, sensor_id)
        if sd is None:
            out = (np.array([]), np.empty((0, 0)))
            self._det2d_cache[cache_key] = out
            return out
        scan_idx = sd.get("scan_index", np.array([]))
        sig_dict = sd.get("detection_signals", {}).get(signal_name, {})
        if not sig_dict:
            out = (scan_idx, np.empty((len(scan_idx), 0)))
            self._det2d_cache[cache_key] = out
            return out
        max_det = max(sig_dict.keys())
        n = len(scan_idx)
        arr = np.full((n, max_det), np.nan)
        for det_idx, vals in sig_dict.items():
            col = det_idx - 1
            length = min(len(vals), n)
            arr[:length, col] = vals[:length]
        out = (scan_idx, arr)
        self._det2d_cache[cache_key] = out
        return out

    def get_alignment_signals(
        self, label: str, sensor_id: str
    ) -> Dict[str, np.ndarray]:
        sd = self.get_sensor(label, sensor_id)
        if sd is None:
            return {}
        return sd.get("alignment_signals", {})

    def get_header_signals(
        self, label: str, sensor_id: str
    ) -> Dict[str, np.ndarray]:
        sd = self.get_sensor(label, sensor_id)
        if sd is None:
            return {}
        return sd.get("header_signals", {})

    def get_all_signal_names(self, label: str, sensor_id: str) -> List[str]:
        """Return sorted list of all detection signal base names for a sensor."""
        sd = self.get_sensor(label, sensor_id)
        if sd is None:
            return []
        return sorted(sd.get("detection_signals", {}).keys())

    def summary(self) -> str:
        """Return human-readable summary of stored data."""
        lines = []
        for label, sensors in self._stores.items():
            lines.append(f"[{label}]")
            for sid, sd in sensors.items():
                det_sigs = list(sd.get("detection_signals", {}).keys())
                lines.append(
                    f"  {sid} ({sd.get('friendly_name')}): "
                    f"{sd.get('n_scans', 0)} scans, "
                    f"{sd.get('max_det_index', 0)} max det, "
                    f"{len(det_sigs)} signal types"
                )
        return "\n".join(lines)

    # ── Comparison API (input vs output) ────────────────────────────────
    def compare_detection_signal(
        self,
        sensor_id: str,
        signal_name: str,
        input_label: str = "input",
        output_label: str = "output",
    ) -> Dict[str, Any]:
        """
        Compare a detection signal between input and output for a sensor.

        Returns dict with keys:
          scan_index, input_2d, output_2d, diff_2d, common_scan_idx
        """
        in_sd = self.get_sensor(input_label, sensor_id)
        out_sd = self.get_sensor(output_label, sensor_id)
        result: Dict[str, Any] = {}

        if in_sd is None and out_sd is None:
            return result

        in_scan = in_sd.get("scan_index", np.array([])) if in_sd else np.array([])
        out_scan = out_sd.get("scan_index", np.array([])) if out_sd else np.array([])

        # Find common scans
        common = np.intersect1d(in_scan, out_scan) if len(in_scan) and len(out_scan) else np.array([])
        result["common_scan_index"] = common
        result["input_scan_index"] = in_scan
        result["output_scan_index"] = out_scan

        # Build 2D arrays for input and output on common scans
        for lbl, sd, key in [
            (input_label, in_sd, "input_2d"),
            (output_label, out_sd, "output_2d"),
        ]:
            if sd is None:
                result[key] = np.empty((0, 0))
                continue
            sig = sd.get("detection_signals", {}).get(signal_name, {})
            if not sig:
                result[key] = np.empty((0, 0))
                continue
            max_det = max(sig.keys())
            scan_arr = sd.get("scan_index", np.array([]))
            # map scan values → row index
            scan_to_row = {int(v): i for i, v in enumerate(scan_arr)}
            n_common = len(common)
            arr = np.full((n_common, max_det), np.nan)
            for ci, sv in enumerate(common):
                ri = scan_to_row.get(int(sv))
                if ri is None:
                    continue
                for det_idx, vals in sig.items():
                    col = det_idx - 1
                    if ri < len(vals):
                        arr[ci, col] = vals[ri]
            result[key] = arr

        # Compute difference if both are present
        if result.get("input_2d") is not None and result.get("output_2d") is not None:
            in2 = result["input_2d"]
            out2 = result["output_2d"]
            if in2.shape == out2.shape and in2.size > 0:
                result["diff_2d"] = in2 - out2

        return result
