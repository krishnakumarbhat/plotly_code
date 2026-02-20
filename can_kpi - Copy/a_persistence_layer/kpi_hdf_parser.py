"""KPI HDF parser.

Builds structured per-sensor dicts from attribute-based CAN-radar HDF5 files.
"""

import logging
from typing import Any, Dict

import numpy as np

from a_persistence_layer.hdf_wrapper import HdfAttrReader

logger = logging.getLogger(__name__)


class KpiHdfParser:
    """High-level parser that converts an HDF file to per-sensor dictionaries."""

    KPI_DETECTION_SIGNALS = [
        "DET_AZIMUTH",
        "DET_ELEVATION",
        "DET_RANGE",
        "DET_RANGE_VELOCITY",
        "DET_SNR",
        "DET_RCS",
        "DET_EXISTENCE_PROB",
        "DET_AZIMUTH_ERR",
        "DET_ELEVATION_ERR",
        "DET_RANGE_ERR",
        "DET_RANGE_VELOCITY_ERR",
    ]

    SCAN_GROUP_SIZE = 4

    def __init__(self, reader: HdfAttrReader | None = None):
        self._reader = reader or HdfAttrReader()

    def parse_file(self, hdf_path: str) -> Dict[str, Any]:
        raw = self._reader.read_hdf_attrs(hdf_path)
        parsed: Dict[str, Any] = {}
        for sensor_id, sensor_data in raw.items():
            scan_idx = self._reader.get_scan_index(sensor_data)
            det_signals = self._reader.extract_detection_signals(sensor_data)

            if len(scan_idx) == 0 and not det_signals:
                logger.info(f"Sensor {sensor_id}: no scan index and no detections — skipping")
                continue

            if not det_signals:
                logger.info(f"Sensor {sensor_id}: no detection signals (keeping header/alignment if present)")

            parsed[sensor_id] = {
                "friendly_name": sensor_data.get("friendly_name", sensor_id),
                "scan_index": scan_idx,
                "n_scans": int(len(scan_idx)),
                "max_det_index": self._max_det_index(det_signals),
                "detection_signals": det_signals,
                "alignment_signals": self._reader.extract_alignment_signals(sensor_data),
                "header_signals": self._reader.extract_header_signals(sensor_data),
            }

            logger.info(
                f"Parsed {sensor_id}: {len(scan_idx)} scans, "
                f"{len(det_signals)} signal types, max det idx {parsed[sensor_id]['max_det_index']}"
            )
        return parsed

    def build_scan_group_dict(
        self, det_signals: Dict[str, Dict[int, np.ndarray]], scan_index: np.ndarray
    ) -> Dict[str, np.ndarray]:
        if not det_signals or len(scan_index) == 0:
            return {}
        n_scans = len(scan_index)
        out: Dict[str, np.ndarray] = {}
        for sig_name, idx_map in det_signals.items():
            if not idx_map:
                continue
            max_idx = max(idx_map.keys())
            arr = np.full((n_scans, max_idx), np.nan)
            for det_idx, values in idx_map.items():
                col = det_idx - 1
                length = min(len(values), n_scans)
                arr[:length, col] = values[:length]
            out[sig_name] = arr
        return out

    def _max_det_index(self, det_signals: Dict[str, Dict[int, np.ndarray]]) -> int:
        m = 0
        for idx_map in det_signals.values():
            if idx_map:
                m = max(m, max(idx_map.keys()))
        return int(m)
