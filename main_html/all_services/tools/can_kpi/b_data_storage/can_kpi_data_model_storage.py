from typing import Any, Dict, List

import numpy as np


class KPI_DataModelStorage:
    """Scan-index keyed hierarchical storage used by HDF parser."""

    def __init__(self):
        self._value_to_signal: Dict[str, str] = {}
        self._signal_to_value: Dict[str, Any] = {}
        self._data_container: Dict[int, List[List[Any]]] = {}
        self._missing_indices = set()
        self._parent_counter = -1
        self._child_counter = -1
        self.stream_name = ""

    def initialize(self, scan_index, sensor, missing_idx=None) -> None:
        self._missing_indices = set(missing_idx) if missing_idx is not None else set()
        self._data_container = {int(scan): [] for scan in scan_index}

    def init_parent(self, stream_name: str) -> None:
        self._parent_counter += 1
        self._child_counter = -1
        self.stream_name = stream_name

    def set_value(self, dataset: Any, signal_name: str, grp_name: str) -> str:
        dataset_len = len(dataset) if dataset is not None else 0
        if dataset_len != len(self._data_container):
            return ""

        is_new_parent = (
            grp_name not in self._signal_to_value and self._child_counter == -1
        )
        if is_new_parent:
            key_grp = f"{self._parent_counter}_None"
            self._child_counter = 0
            key = f"{self._parent_counter}_{self._child_counter}"
            for row, scanidx in zip(dataset, self._data_container):
                self._data_container[scanidx].append([self._norm_row(row)])
            self._value_to_signal[key] = signal_name
            self._value_to_signal[key_grp] = self.stream_name
            self._signal_to_value[signal_name] = key
            self._signal_to_value[self.stream_name] = key_grp
            return key

        self._child_counter += 1
        key = f"{self._parent_counter}_{self._child_counter}"
        for row, scanidx in zip(dataset, self._data_container):
            self._data_container[scanidx][-1].append(self._norm_row(row))

        self._value_to_signal[key] = signal_name
        current = self._signal_to_value.get(signal_name)
        if current is None:
            self._signal_to_value[signal_name] = [{grp_name: key}]
        elif isinstance(current, list):
            current.append({grp_name: key})
        else:
            self._signal_to_value[signal_name] = [{grp_name: key}]
        return key

    def clear(self) -> None:
        self._value_to_signal.clear()
        self._signal_to_value.clear()
        self._data_container.clear()
        self._parent_counter = -1
        self._child_counter = -1

    def _norm_row(self, row: Any) -> Any:
        if isinstance(row, np.ndarray):
            return np.round(row.astype(float), 2)
        return row
