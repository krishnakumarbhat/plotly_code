from __future__ import annotations

from pathlib import Path
from typing import Any

import h5py


class HdfSnapshotRepository:
    def create_snapshot(self, file_path: str) -> dict[str, Any]:
        path = Path(file_path)
        if not path.exists():
            return {
                "exists": False,
                "file": str(path),
                "sensors": [],
                "datasets": {},
                "error": "file_not_found",
            }

        datasets: dict[str, dict[str, Any]] = {}
        sensors: set[str] = set()

        with h5py.File(path, "r") as handle:
            for key in handle.keys():
                sensors.add(str(key))

            def visitor(name: str, obj: Any) -> None:
                if isinstance(obj, h5py.Dataset):
                    datasets[name] = {
                        "shape": [int(x) for x in obj.shape],
                        "dtype": str(obj.dtype),
                        "size": int(obj.size),
                    }

            handle.visititems(visitor)

        return {
            "exists": True,
            "file": str(path),
            "sensors": sorted(sensors),
            "datasets": datasets,
        }

    def compare_snapshots(self, input_snapshot: dict[str, Any], output_snapshot: dict[str, Any]) -> dict[str, Any]:
        input_ds = input_snapshot.get("datasets", {})
        output_ds = output_snapshot.get("datasets", {})

        input_paths = set(input_ds.keys())
        output_paths = set(output_ds.keys())
        common_paths = sorted(input_paths & output_paths)

        shape_matches = 0
        dtype_matches = 0
        for path in common_paths:
            in_meta = input_ds[path]
            out_meta = output_ds[path]
            if in_meta.get("shape") == out_meta.get("shape"):
                shape_matches += 1
            if in_meta.get("dtype") == out_meta.get("dtype"):
                dtype_matches += 1

        total_common = len(common_paths)
        shape_match_pct = (100.0 * shape_matches / total_common) if total_common else 0.0
        dtype_match_pct = (100.0 * dtype_matches / total_common) if total_common else 0.0

        return {
            "input_dataset_count": len(input_paths),
            "output_dataset_count": len(output_paths),
            "common_dataset_count": total_common,
            "input_only_count": len(input_paths - output_paths),
            "output_only_count": len(output_paths - input_paths),
            "shape_match_pct": round(shape_match_pct, 2),
            "dtype_match_pct": round(dtype_match_pct, 2),
            "input_sensor_count": len(input_snapshot.get("sensors", [])),
            "output_sensor_count": len(output_snapshot.get("sensors", [])),
            "input_sensors": input_snapshot.get("sensors", []),
            "output_sensors": output_snapshot.get("sensors", []),
        }
