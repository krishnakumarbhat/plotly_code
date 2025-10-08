import numpy as np
import pandas as pd

from aspe.utilities.nearest_neigbor_sync import sync_by_nearest_neighbor
from aspe.utilities.SupportingFunctions import cast_enum_columns_to_ints, cast_int_columns_to_enums


def interpolate_floats(arr_in: np.ndarray, old_timestamp: np.ndarray, new_timestamp: np.ndarray) -> np.ndarray:
    rows_num, columns_num = new_timestamp.shape[0], arr_in.shape[1]
    signals_out = np.zeros((rows_num, columns_num))
    arr_f64 = arr_in.astype(np.float64)
    for col_idx in range(columns_num):
        signal_vals = arr_f64[:, col_idx]
        signal_interp = np.interp(x=new_timestamp, xp=old_timestamp, fp=signal_vals)
        signals_out[:, col_idx] = signal_interp
    return signals_out


def interpolate_angular_values(arr_in: np.ndarray, old_timestamp: np.ndarray, new_timestamp: np.ndarray) -> np.ndarray:
    rows_num, columns_num = new_timestamp.shape[0], arr_in.shape[1]
    signals_out = np.zeros((rows_num, columns_num))
    arr_f64 = arr_in.astype(np.float64)
    for col_idx in range(columns_num):
        signal_vals = arr_f64[:, col_idx]
        signal_unwrap = np.unwrap(signal_vals)
        signal_interp = np.interp(x=new_timestamp, xp=old_timestamp, fp=signal_unwrap)
        signal_wrap = (signal_interp + np.pi) % (2 * np.pi) - np.pi  # wrap to [-pi, + pi]
        signals_out[:, col_idx] = signal_wrap
    return signals_out


def interpolate_ints(arr_in: np.ndarray, old_timestamp: np.ndarray, new_timestamp: np.ndarray) -> np.ndarray:
    return sync_by_nearest_neighbor(arr_in, old_timestamp, new_timestamp, dtype=int)[0]


def interpolate_enums(arr_in: np.ndarray, old_timestamp: np.ndarray, new_timestamp: np.ndarray) -> np.ndarray:
    enums_as_ints, enum_col_to_enum_class_map = cast_enum_columns_to_ints(arr_in)
    synced, _ = sync_by_nearest_neighbor(enums_as_ints, old_timestamp, new_timestamp, dtype=object)
    out = cast_int_columns_to_enums(synced, enum_col_to_enum_class_map)
    return out


def interpolate_bools(arr_in: np.ndarray, old_timestamp: np.ndarray, new_timestamp: np.ndarray) -> np.ndarray:
    df_as_ints = arr_in.astype(int)
    synced, _ = sync_by_nearest_neighbor(df_as_ints, old_timestamp, new_timestamp, dtype=bool)
    return synced.astype(bool)


def interpolate_lists(arr_in: np.ndarray, old_timestamp: np.ndarray, new_timestamp: np.ndarray) -> np.ndarray:
    columns_count = arr_in.shape[1]
    rows_count = new_timestamp.shape[0]
    arr_out = np.zeros(shape=(rows_count, columns_count), dtype=object)
    for col_idx in range(columns_count):
        sub_arr = np.vstack(arr_in[:, col_idx])
        interp_sub_arr = interpolate_floats(sub_arr, old_timestamp, new_timestamp)
        for row_idx in range(rows_count):
            row = interp_sub_arr[row_idx, :]
            arr_out[row_idx, col_idx] = row.tolist()
    return arr_out