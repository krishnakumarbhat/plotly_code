import warnings
from typing import Tuple

import numpy as np
import pandas as pd

from aspe.utilities.SupportingFunctions import get_from_dict_by_map, recursive_dict_extraction


def flatten_someip_signals(some_ip_data: dict, data_key: str) -> dict:
    """
    Transforms object list signals  in form of nested dictionaries into dictionary of flatten 2d arrays.
    :param some_ip_data: nested dictionary extracted from someip bus containing object list data
    :param data_key: name of key within main data is stored
    :return: dict of numpy ndarrays where each array represents signal values, number of columns is equal to maximum
    number of objects and number of rows is equal to number of scans within log file
    """
    vigem_ts = list(some_ip_data.keys())

    single_scan = some_ip_data[vigem_ts[0]]
    single_object = single_scan[data_key][0]

    number_of_si = len(vigem_ts)
    number_of_objects = len(single_scan[data_key])

    signals_dict, signals_map = initialize_signals_dict(single_object, number_of_si, number_of_objects)
    for signature, array in signals_dict.items():
        curr_map = signals_map[signature]
        for row_index, (ts, single_scan_objects) in enumerate(some_ip_data.items()):
            for column_index, single_object in enumerate(single_scan_objects[data_key]):
                signal_value = get_from_dict_by_map(single_object, curr_map)
                try:
                    signals_dict[signature][row_index, column_index] = signal_value
                except ValueError:
                    warnings.warn(f'Wrong type of {signature}, expected {signals_dict[signature].dtype} ')
    return signals_dict


def flatten_someip_headers(input_data: dict, keys_to_skip=tuple()) -> dict:
    """
    Transforms some ip data header in form of nested dictionaries into dictionary of flatten 2d arrays.
    :param input_data: nested dictionary extracted from someip bus
    :param keys_to_skip: tuple of keys which should be excluded from recursive dict extraction
    :return: dict of numpy ndarrays where each array represents header structure values, each array contains signle
    column and number of rows is equal to number of scan within log file
    """
    vigem_ts = list(input_data.keys())
    single_scan = input_data[vigem_ts[0]]
    number_of_si = len(vigem_ts)
    header_dict, header_map = initialize_headers_dict(single_scan, number_of_si, keys_to_skip=keys_to_skip)

    for signature, array in header_dict.items():
        curr_map = header_map[signature]
        for row_index, (ts, single_scan_objects) in enumerate(input_data.items()):
            try:
                header_dict[signature][row_index] = get_from_dict_by_map(single_scan_objects, curr_map)
            except ValueError:
                warnings.warn(f'Wrong type of {signature}, expected {header_dict[signature].dtype}')
            except OverflowError:
                warnings.warn(f'Overflow issue with signal: {signature}')
    header_dict['vigem_ts'] = np.array(vigem_ts)
    return header_dict


def filter_out_empty_slots(sensor_data_flat, number_of_detections):
    mask = np.vstack([
        np.array([np.full(nod, i), np.arange(nod)]).T
        for i, nod in enumerate(number_of_detections)
    ])
    rows = mask[:, 0]
    columns = mask[:, 1]

    signals_df = pd.DataFrame()
    for signal_name, signal_values in sensor_data_flat.items():
        signals_df.loc[:, signal_name] = signal_values[rows, columns]

    signals_df['row'] = rows
    signals_df['column'] = columns
    return signals_df


def get_correct_dtype(value):
    if isinstance(value, int):
        value_dtype = np.int64
    elif isinstance(value, str):
        value_dtype = object
    else:
        value_dtype = type(value)
    return value_dtype


def initialize_signals_dict(single_object, number_of_si, number_of_objects) -> Tuple[dict, dict]:
    """
    Initializes dictionary with flattened key names of object list signals containing empty numpy.ndarrays.
    :param single_object: dictionary with single object within single scan index data
    :param number_of_si: number of total scan index in log
    :param number_of_objects: number of total objects in log
    :return: tuple of two dicts - first contains flatted, empty structure, second is dictionary of lists with keys to
    which are used to get nested signals - it connects raw input dictionary and flattened output
    """
    out_structure = dict()
    extracted, out_map = recursive_dict_extraction(single_object)
    for key, value in extracted.items():
        value_dtype = get_correct_dtype(value)
        out_structure[key] = np.empty(shape=(number_of_si, number_of_objects), dtype=value_dtype)
    return out_structure, out_map


def initialize_headers_dict(single_scan: dict, number_of_si: int, keys_to_skip=tuple()) -> Tuple[dict, dict]:
    """
    Initializes dictionary with flattened key names of object list metadata containing empty numpy.ndarrays.
    :param single_scan: dictionary with single scan index data
    :param number_of_si: number of total scan index in log
    :return: tuple of two dicts - first contains flatted, empty structure, second is dictionary of lists with keys to
    which are used to get nested signals - it connects raw input dictionary and flattened output
    """
    out_structure = dict()
    extracted, out_map = recursive_dict_extraction(single_scan, keys_to_skip=keys_to_skip)
    for key, value in extracted.items():
        value_dtype = get_correct_dtype(value)
        out_structure[key] = np.empty(shape=number_of_si, dtype=value_dtype)
    return out_structure, out_map


if __name__ == '__main__':
    from time import time

    from mdf_parser_pkg.mdf_parser import parse_file
    from mdf_parser_pkg.srr5_dev_tools.mgp_module import load as load_mgp
    t1 = time()

    #parsed_data = parse_file(r"C:\logs\mdf_with_rt_range\TC3_SRR_REFERENCE_WBATR91070LC63638_20191211_130228_ref_0002.MF4", 'mid', save_file=True)
    parsed_data = load_mgp(r"C:\logs\DEX_497\DEX_475\BN_FASETH\parse_result\20200128T155737_20200128T155757_543078_LB36408_BN_FASETH_SRR_Master-mPAD.mgp")
    t2 = time()

    object_list_data = parsed_data['RecogSideRadarObjectlist']['Objectlist']
    signals = flatten_someip_signals(object_list_data)
    headers = flatten_someip_headers(object_list_data)
    t3 = time()
    print(f"parsing time: {t2-t1}")
    print(f"transform time: {t3-t2}")


