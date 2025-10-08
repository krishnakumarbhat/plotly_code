import warnings
from typing import Tuple

import numpy as np
import pandas as pd

from aspe.utilities.SupportingFunctions import get_from_dict_by_map, recursive_dict_extraction


def flatten_someip_object_list_signals(object_list_data: dict) -> dict:
    """
    Transforms object list signals  in form of nested dictionaries into dictionary of flatten 2d arrays.
    :param object_list_data: nested dictionary extracted from someip bus containing object list data
    :return: dict of numpy ndarrays where each array represents signal values, number of columns is equal to maximum
    number of objects and number of rows is equal to number of scans within log file
    """
    obj_list_key = 'object_list'
    vigem_ts = list(object_list_data.keys())

    single_scan = object_list_data[vigem_ts[0]]
    single_object = single_scan[obj_list_key][0]

    number_of_si = len(vigem_ts)
    number_of_objects = len(single_scan[obj_list_key])

    signals_dict, signals_map = initialize_signals_dict(single_object, number_of_si, number_of_objects)
    for signature, array in signals_dict.items():
        curr_map = signals_map[signature]
        for row_index, (ts, single_scan_objects) in enumerate(object_list_data.items()):
            for column_index, single_object in enumerate(single_scan_objects[obj_list_key]):
                signal_value = get_from_dict_by_map(single_object, curr_map)
                try:
                    signals_dict[signature][row_index, column_index] = signal_value
                except ValueError:
                    warnings.warn(f'Wrong type of {signature}, expected {signals_dict[signature].dtype} ')
    return signals_dict


def flatten_someip_object_list_headers(object_list_data: dict) -> dict:
    """
    Transforms object list header data in form of nested dictionaries into dictionary of flatten 2d arrays.
    :param object_list_data: nested dictionary extracted from someip bus containing object list data
    :return: dict of numpy ndarrays where each array represents header structure values, each array contains signle
    column and number of rows is equal to number of scan within log file
    """
    vigem_ts = list(object_list_data.keys())
    single_scan = object_list_data[vigem_ts[0]]
    number_of_si = len(vigem_ts)
    metadata_dict, metadata_map = initialize_headers_dict(single_scan, number_of_si)

    for signature, array in metadata_dict.items():
        curr_map = metadata_map[signature]
        for row_index, (ts, single_scan_objects) in enumerate(object_list_data.items()):
            try:
                metadata_dict[signature][row_index] = get_from_dict_by_map(single_scan_objects, curr_map)
            except ValueError:
                warnings.warn(f'Wrong type of {signature}, expected {metadata_dict[signature].dtype}')
            except OverflowError:
                warnings.warn(f'Overflow issue with signal: {signature}')
    metadata_dict['vigem_ts'] = np.array(vigem_ts)
    return metadata_dict


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


def initialize_headers_dict(single_scan: dict, number_of_si: int) -> Tuple[dict, dict]:
    """
    Initializes dictionary with flattened key names of object list metadata containing empty numpy.ndarrays.
    :param single_scan: dictionary with single scan index data
    :param number_of_si: number of total scan index in log
    :return: tuple of two dicts - first contains flatted, empty structure, second is dictionary of lists with keys to
    which are used to get nested signals - it connects raw input dictionary and flattened output
    """
    out_structure = dict()
    extracted, out_map = recursive_dict_extraction(single_scan)
    for sensor_idx, sensor_ts_dict in enumerate(single_scan['timestamp_detections']):
        sensor_ts_dets, sensor_ts_dets_map = recursive_dict_extraction(sensor_ts_dict)
        for dets_ts_key, dets_ts_value in sensor_ts_dets.items():
            new_key = f'timestamp_detections_{sensor_idx}_{dets_ts_key}'
            extracted[new_key] = dets_ts_value
            out_map[new_key] = ['timestamp_detections', sensor_idx] + sensor_ts_dets_map[dets_ts_key]

    for key, value in extracted.items():
        value_dtype = get_correct_dtype(value)
        out_structure[key] = np.empty(shape=number_of_si, dtype=value_dtype)
    return out_structure, out_map


def flatten_rt_range_parsed_data(parsed_file):
    '''
    Flattens raw RT Range data grouping messages separating Host and Target
    :param parsed_file: raw RT Range parsed data
    :return: nested dictionary of extracted signals
    '''
    # loop over signals and subsignals in parsed_file and add
    data_dict = {}
    o1_key = 'Target1'
    o2_key = 'Target2'
    h_key = 'Host'
    data_dict = {}
    parsed_file = {k: v for k, v in parsed_file.items() if isinstance(v, dict)}
    for signal in parsed_file.keys():

        # choose if the signal belongs to object or host subdict
        if ('Range' in signal or 'Target' in signal) and '2' not in signal:
            key = o1_key
        elif ('Range' in signal or 'Target' in signal) and '2' in signal:
            key = o2_key
        else:
            key = h_key

        if key not in data_dict.keys():
            data_dict[key] = {}

        tstamp_signalname = signal + '_timestamp'

        data_dict[key][signal] = {}
        for subsig_ix, subsignal in enumerate(parsed_file[signal].keys()):
            # skip subsignal _id since not relevant
            if subsignal == '_id':
                continue

            # create subsignal array
            data_dict[key][signal][subsignal] = np.array([])

            # create one timestamp array per signal
            first_subsignal = subsig_ix == 1
            if first_subsignal:
                data_dict[key][signal][tstamp_signalname] = np.array([])

            for val in parsed_file[signal][subsignal]:
                data_dict[key][signal][subsignal] = np.concatenate([data_dict[key][signal][subsignal],
                                                                    [val[0]]])
                if first_subsignal:
                    data_dict[key][signal][tstamp_signalname] = \
                        np.concatenate([data_dict[key][signal][tstamp_signalname], [val[1]]])

    return data_dict


def mdf_rt_extract_utc(signal_dataframe: pd.DataFrame, mapping_utc_signals: str) -> pd.Series:
    """
    Function mainly dedicated for RtMdfExtractor!
    Wrapped function to not repeating code
    :param signal_dataframe:  dataframe filled with signals
    :param source: Source of UTC extracion ("Host", "Target1")
    :return: pandas series of utc. returning Nan when some signals are missing
    """

    utc_signals_names = \
        [
            'century',
            'year',
            'month',
            'day',
            'hour',
            'minute',
            'seconds',
            'ms',
        ]
    try:
        target_time = signal_dataframe.loc[:, mapping_utc_signals]
        target_time.columns = utc_signals_names

        target_time.loc[:, 'century'] *= 100  # century in Rt are represent as for e.g. 20 instead of 2000
        target_time.loc[:, 'ms'] *= 1000  # despite of 'ms' name Rt provides it in seconds (for e.g. 0.19)
        target_time.loc[:, 'year'] += target_time['century']

        target_time = target_time.drop(columns=['century'])
        target_utc_time = pd.to_datetime(target_time)
    except KeyError:
        target_utc_time = np.nan
        warnings.warn('cant find some signals for utc parsing... skipping this part')

    return target_utc_time


def align_rt_messages(flattened_data, master_message):
    """
    Aligns RT messages to master message (message always sent first)
    :param flattened_data: flat dictionary of rt signals
    :param master_message: name of master_message
    :return: flat dictionary with missing signals value filled with nan
    """

    ts_arrays = {k: v for k, v in flattened_data.items() if '_timestamp' in k}
    master_ts = ts_arrays.pop(master_message)
    msg_sets_indexes = {k: [] for k in ts_arrays.keys()}
    length_of_master = len(master_ts)

    for master_ts_idx in range(0, length_of_master):
        curr_master_ts = master_ts[master_ts_idx]
        if master_ts_idx < length_of_master - 1:
            next_master_ts = master_ts[master_ts_idx + 1]
        else:
            next_master_ts = np.inf

        for msg_set_signature, msg_set_ts in ts_arrays.items():
            curr_msg_set_index = np.where((curr_master_ts < msg_set_ts) & (msg_set_ts < next_master_ts))[0]
            if len(curr_msg_set_index) > 0:
                msg_sets_indexes[msg_set_signature].append(curr_msg_set_index[0])
            else:
                msg_sets_indexes[msg_set_signature].append(np.nan)

    msg_sets_indexes[master_message] = list(range(len(master_ts)))
    output = {}
    for msg_group_ts_name, indexes in msg_sets_indexes.items():
        msg_group_name = msg_group_ts_name.split('_')[0]
        msgs_to_align = {k: v for k, v in flattened_data.items() if msg_group_name in k}
        for msg_name, msg_vals in msgs_to_align.items():
            output[msg_name] = extract_from_indexes_with_nan(msg_vals, indexes)
    return output


def extract_from_indexes_with_nan(array, indexes):
    """
    Extracts value from array for integers in indexes, inserts nan for each nan in indexes
    :param array: array from which values are being extracted
    :param indexes: array with indexes of values to extract
    :return: array with extracted values for integers indexes, filled with nan for each nan in indexes
    """
    return np.array([array[idx] if not np.isnan(idx) and idx < len(array) else np.nan for idx in indexes])


def find_master_msg(raw_data: dict, vehicle_type: str):

    def extract_time_diff(msg):
        time = np.array([x[1] for x in msg])
        mean_time_diff = np.mean(np.diff(time))
        min_ts = min(time)
        return mean_time_diff, min_ts

    msg_data = {}
    if vehicle_type == "Host":
        msg_set = {k: v for k, v in raw_data.items() if 'Target' not in k and 'Range' not in k and isinstance(v, dict)}
    elif vehicle_type == 'Target':
        msg_set = {k: v for k, v in raw_data.items() if 'Target' in k or 'Range' in k and isinstance(v, dict)}

    for name, msgs in msg_set.items():
        id = msgs['_id']
        for nested_name, msg in msgs.items():
            if nested_name != '_id':
                mean_time_diff, min_ts = extract_time_diff(msg)
                msg_data.update({id: {'name': name, 'min_ts': min_ts, 'mean_time_diff': mean_time_diff}})
    master_id = min({k: v for k, v in msg_data.items() if 0.009 < v['mean_time_diff'] < 0.011}.keys())
    return msg_data[master_id]['name']


def get_full_master_name(msg_names, name):
    return [x for x in msg_names if name in x and '_timestamp' in x][0]
