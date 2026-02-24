"""HDF5 reader for CAN KPI project.

CAN HDF5 files store data as group attributes (not datasets).
"""

import logging
import re
from typing import Any, Dict, List

import h5py
import numpy as np

logger = logging.getLogger(__name__)


class HdfAttrReader:
    """Loads a CAN-radar HDF5 file into nested dicts."""

    SENSOR_NAME_MAP = {
        "CEER_FL": "Front Left (SRR_FL)",
        "CEER_FLR": "Front Long Range (FLR)",
        "CEER_FR": "Front Right (SRR_FR)",
        "CEER_RL": "Rear Left (SRR_RL)",
        "CEER_RR": "Rear Right (SRR_RR)",
    }

    def read_hdf_attrs(self, path: str) -> Dict[str, Any]:
        result: Dict[str, Any] = {}
        with h5py.File(path, "r") as f:
            for sensor_id in f.keys():
                sensor_grp = f[sensor_id]
                if not isinstance(sensor_grp, h5py.Group):
                    continue

                sensor_data: Dict[str, Any] = {
                    "friendly_name": self.SENSOR_NAME_MAP.get(sensor_id, sensor_id)
                }
                classified = self._classify_subgroups(list(sensor_grp.keys()))
                for category, names in classified.items():
                    sensor_data[category] = {
                        gname: self._read_group_attrs(sensor_grp[gname])
                        for gname in names
                        if isinstance(sensor_grp[gname], h5py.Group)
                        and len(sensor_grp[gname].attrs.keys()) > 0
                    }
                result[sensor_id] = sensor_data
        return result

    def read_hdf_collect(self, path: str) -> Dict[str, Any]:
        """Legacy dataset reader (kept for compatibility; unused for CAN attr-style HDF)."""
        data: Dict[str, Any] = {}
        with h5py.File(path, "r") as f:

            def visitor(name, obj):
                if isinstance(obj, h5py.Dataset):
                    try:
                        arr = obj[()]
                        data[name] = np.array([arr]) if np.isscalar(arr) else arr
                    except Exception:
                        pass

            f.visititems(visitor)
        return data

    def get_scan_index(self, sensor_data: Dict[str, Any]) -> np.ndarray:
        header_groups = sensor_data.get("header", {})
        for attrs in header_groups.values():
            for key in ("HED_SCAN_INDEX", "HED_LOOK_INDEX"):
                if key in attrs:
                    return attrs[key].astype(int)

        det_groups = sensor_data.get("detection", {})
        scan_from_names = self._scan_index_from_detection_group_names(
            list(det_groups.keys())
        )
        if len(scan_from_names) > 0:
            return scan_from_names

        for attrs in det_groups.values():
            for val in attrs.values():
                if isinstance(val, np.ndarray) and val.ndim == 1:
                    return np.arange(1, len(val) + 1)
        return np.array([])

    def _scan_index_from_detection_group_names(
        self, group_names: List[str]
    ) -> np.ndarray:
        ranges: List[tuple[int, int]] = []
        for name in group_names:
            match = re.search(r"_(\d{3})_(\d{3})$", name)
            if not match:
                continue
            start_idx = int(match.group(1))
            end_idx = int(match.group(2))
            if end_idx < start_idx:
                continue
            ranges.append((start_idx, end_idx))

        if not ranges:
            return np.array([])

        values = set()
        for start_idx, end_idx in ranges:
            values.update(range(start_idx, end_idx + 1))
        return np.array(sorted(values), dtype=int)

    def extract_detection_signals(
        self, sensor_data: Dict[str, Any]
    ) -> Dict[str, Dict[int, np.ndarray]]:
        det_groups = sensor_data.get("detection", {})
        signals: Dict[str, Dict[int, np.ndarray]] = {}
        for attrs in det_groups.values():
            real_attrs = {
                k: v
                for k, v in attrs.items()
                if not k.startswith("id_") and not k.startswith("timestamp_")
            }
            if not real_attrs:
                continue
            for attr_name, arr in real_attrs.items():
                m = re.match(r"^(.+?)_(\d{3})$", attr_name)
                if not m:
                    continue
                prefix, det_idx = m.group(1), int(m.group(2))
                signals.setdefault(prefix, {})[det_idx] = arr
        return signals

    def extract_alignment_signals(
        self, sensor_data: Dict[str, Any]
    ) -> Dict[str, np.ndarray]:
        return self._extract_flat(sensor_data.get("alignment", {}))

    def extract_header_signals(
        self, sensor_data: Dict[str, Any]
    ) -> Dict[str, np.ndarray]:
        return self._extract_flat(sensor_data.get("header", {}))

    def _read_group_attrs(self, grp: h5py.Group) -> Dict[str, np.ndarray]:
        out: Dict[str, np.ndarray] = {}
        for k in grp.attrs.keys():
            v = grp.attrs[k]
            out[k] = v if isinstance(v, np.ndarray) else np.array(v)
        return out

    def _extract_flat(
        self, groups: Dict[str, Dict[str, np.ndarray]]
    ) -> Dict[str, np.ndarray]:
        out: Dict[str, np.ndarray] = {}
        for attrs in groups.values():
            for k, v in attrs.items():
                if not k.startswith("id_") and not k.startswith("timestamp_"):
                    out[k] = v
        return out

    def _classify_subgroups(self, subgroup_names: List[str]) -> Dict[str, List[str]]:
        cats: Dict[str, List[str]] = {
            "detection": [],
            "alignment": [],
            "header": [],
            "status": [],
            "capability": [],
            "other": [],
        }
        for name in sorted(subgroup_names):
            u = name.upper()
            if "DETECTION" in u:
                cats["detection"].append(name)
            elif "ALIGNMENT" in u:
                cats["alignment"].append(name)
            elif "HEADER" in u:
                cats["header"].append(name)
            elif "STATUS" in u:
                cats["status"].append(name)
            elif "CAPABILITY" in u:
                cats["capability"].append(name)
            else:
                cats["other"].append(name)
        return cats
