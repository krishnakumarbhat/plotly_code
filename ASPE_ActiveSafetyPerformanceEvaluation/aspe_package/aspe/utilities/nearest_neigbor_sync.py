import numpy as np
import pandas as pd


def sync_by_nearest_neighbor(arr_in: np.ndarray, old_timestamp: np.ndarray, new_timestamp: np.ndarray, dtype=np.float32) -> np.ndarray:
    ts_diff_mat = new_timestamp - old_timestamp.reshape((-1, 1))
    min_diffs_indexes = np.abs(ts_diff_mat).argmin(axis=0)
    if len(arr_in.shape) == 2:
        columns_count = arr_in.shape[1]
        signals_out = np.zeros((new_timestamp.shape[0], columns_count), dtype=dtype)
        for column_idx in range(columns_count):
            signal_vals = arr_in[:, column_idx]
            signals_out[:, column_idx] = signal_vals[min_diffs_indexes]
    elif len(arr_in.shape) == 1:
        signals_out = arr_in[min_diffs_indexes]
    return signals_out, min_diffs_indexes
