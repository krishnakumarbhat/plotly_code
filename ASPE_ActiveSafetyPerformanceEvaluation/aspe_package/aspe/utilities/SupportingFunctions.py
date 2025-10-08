# coding=utf-8
"""
Set of supporting functions for aspe.extractors
"""
import glob
import numbers
import operator
import os
import pathlib
import pickle
from functools import reduce
from pathlib import Path
from time import time
from typing import Iterable, List, Tuple, Union
from warnings import warn

import numpy as np
import pandas as pd


def save_to_pkl(data, output_filename):
    """
    Simple function for saving files to pickle
    :param data: data to save
    :param output_filename: save path with extension
    :return: None
    """
    with open(output_filename, 'wb') as handle:
        pickle.dump(data, handle, protocol=pickle.HIGHEST_PROTOCOL)


def save_todict_pkl(extracted_data, path):
    """
    Save F360ExtractedData as dictionary
    :param extracted_data: data to be saved
    :param path: path to save
    """
    save_to_pkl(extracted_data.to_dict(), path)


def load_from_pkl(path_to_pickled_data):
    """
    Pickle loading function
    :param path_to_pickled_data: Path to pickle file
    :return: Loaded data
    """
    with open(path_to_pickled_data, 'rb') as handle:
        data = pickle.load(handle)
    return data


def get_nested_array(nested_dict, keys):
    """
    Function that returns array from given nested dictionary by using list of keys
    :param nested_dict: nested dictionary
    :param keys: list of keys
    :return: numpy array
    """
    array = nested_dict
    for key in keys:
        if key in array:
            array = array[key]
        else:
            array = None
            break
    return array


def get_from_dict_by_map(data_dict: dict, map_list: List[str]):
    """
    Return value of nested dictionary which can be get by accessing keys stored in map_list.
    TODO replace get_nested_array with this
    :param data_dict: input dictionary
    :param map_list: list of keys
    :return: value stored in data_dict which can be get by accessing keys stored in map_list
    """
    return reduce(operator.getitem, map_list, data_dict)


def get_unique_value_with_most_counts(array, prop_name="", disable_warning=False):
    """
    Get unique value with most counts from property that might have multiple unique values and raise warning if so
    In case of the same number of unique values the smallest value is returned
    :param array: numpy array with single/multiple unique values
    :param prop_name: string, name of the property, default: ""
    :param disable_warning: indication if warning should be disabled, default: false
    :return: unique value with most counts
    """
    unique_values, counts = np.unique(array, return_counts=True)
    value_with_most_counts_idx = 0
    number_of_unique_values = unique_values.shape[0]
    if number_of_unique_values > 1:
        if not disable_warning:
            message = (f'several unique ({number_of_unique_values}) values for property {prop_name} '
                       f'- value with the most counts will be used')
            warn(message)
        value_with_most_counts_idx = np.where(counts == max(counts))[0][0]
    return unique_values[value_with_most_counts_idx]


def get_logs_catalog(catalog_path: str, extension: str) -> List[str]:
    """
    Provides list of logpath from ASPE test catalog
    :param catalog_path: for e.g. "r'\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors'"
    :param extension: for e.g. '.mudp'
    :return: list of logs path
    """
    logs_catalog = []
    for root, dirs, files in os.walk(catalog_path, topdown=False):
        for name in files:
            if extension in name:
                logs_catalog.append(os.path.join(root, name))
    return logs_catalog


def is_1d_array(array):
    """
    Check shape of array and return true if it's 1D array.
    :param array: ndarray
    :return:
    """
    if len(array.shape) == 1:
        return True
    elif len(array.shape) == 2:
        return array.shape[1] == 1
    else:
        return False


def values_are_same(array):
    """
    Check if all array elements are not the same. Compare all elements with first element.
    :param array: ndarray
    :return: bool
    """
    if array.size == 0:
        raise AttributeError('Empty array.')
    return np.all(array.reshape(-1)[0] == array)


def get_nest_till_array_generator(d: dict) -> iter:
    """
    Creating generator from dict with nests as another dicts. While iteration return any nested objects that is not a
    dict.
    Each iteration returns key name str and value which is object other than dict (for e.g. np array)
    :param d: dictionary
    :k: str, key name
    :v: nested objects from dict
    :return: generator
    """
    for k, v in d.items():
        if isinstance(v, dict):
            yield from get_nest_till_array_generator(v)
        else:
            yield k, v


def recursive_dict_extraction(input_data: dict, parent_key="", separator="_", keys_to_skip=tuple()) -> Tuple[
    dict, dict]:
    """
    Recursively extracts data from nested dictionary.
    :param input_data: dictionary to extract data from
    :param parent_key: concatenated key from dictionaries levels above
    :param separator: separator used in concatenating keys
    :return: tuple of two dicts - first contains flatted, empty structure, second is dictionary of lists with keys to
    which are used to get nested signals - it connects raw input dictionary and flattened output
    """

    def recursive_list_tuple_extraction(in_dat: list, out_dat: dict, out_map: dict, curr_map: dict, parent_key: str,
                                        sep: str):
        for index, element in enumerate(in_dat):
            new_key = parent_key + f'{sep}{index}'
            curr_map.append(index)
            if isinstance(element, dict):
                recursive_dict_extraction_internal(element, out_dat, out_map, curr_map, new_key, separator)
            elif isinstance(element, (str, numbers.Number, np.ndarray)):
                out_dat[new_key] = element
                out_map[new_key] = curr_map.copy()
            elif isinstance(element, (list, tuple)):
                recursive_list_tuple_extraction(element, out_dat, out_map, curr_map, new_key, separator)
            curr_map.pop()

    def recursive_dict_extraction_internal(in_dat: dict, out_dat: dict, out_map: dict, curr_map: dict, parent_key: str,
                                           sep: str):
        for key, value in in_dat.items():
            if key in keys_to_skip:
                continue
            new_key = f'{parent_key}{sep}{key}'
            curr_map.append(key)
            if isinstance(value, dict):
                recursive_dict_extraction_internal(value, out_dat, out_map, curr_map, new_key, separator)
            elif isinstance(value, (str, numbers.Number, np.ndarray)):
                out_dat[new_key] = value
                out_map[new_key] = curr_map.copy()
            elif isinstance(value, (list, tuple)):
                recursive_list_tuple_extraction(value, out_dat, out_map, curr_map, new_key, separator)
            else:
                warn(f'Not supported data type: {type(value)} in recursive_dict_extraction_internal. '
                     f'Variable will be excluded from output data')
            curr_map.pop()

    # initialize structures for recursion
    output_data = {}
    out_map = {}
    curr_map = []
    recursive_dict_extraction_internal(input_data, output_data, out_map, curr_map, parent_key, '')

    return output_data, out_map


def get_log_list_from_path(log_general_path,
                           req_ext='.dvl',
                           required_sub_strings=None,
                           restricted_sub_strings=None,
                           level=0):
    log_list = []
    for local_path, dirs, files in walk_level(log_general_path, level=level):
        for file_name in files:
            file_extension = os.path.splitext(file_name)[1]
            if file_extension == req_ext:

                f_in_is_correct_fn = True
                f_out_is_correct_fn = True

                if required_sub_strings is not None:
                    if isinstance(required_sub_strings, str):
                        required_sub_strings = list(required_sub_strings)
                    for single_sub_string in required_sub_strings:
                        if single_sub_string not in file_name and single_sub_string not in local_path:
                            f_in_is_correct_fn = False
                            break

                if restricted_sub_strings is not None:
                    if isinstance(restricted_sub_strings, str):
                        restricted_sub_strings = list(restricted_sub_strings)
                    for single_sub_string in restricted_sub_strings:
                        if single_sub_string in file_name or single_sub_string in local_path:
                            f_out_is_correct_fn = False
                            break

                if f_in_is_correct_fn and f_out_is_correct_fn:
                    path = os.path.join(local_path, file_name)
                    log_list.append(path)
    return log_list


def walk_level(some_dir, level=1):
    some_dir = some_dir.rstrip(os.path.sep)
    assert os.path.isdir(some_dir)
    num_sep = some_dir.count(os.path.sep)
    for root, dirs, files in os.walk(some_dir):
        yield root, dirs, files
        num_sep_this = root.count(os.path.sep)
        if num_sep + level <= num_sep_this:
            del dirs[:]


def get_default_rt_3000_dbc_path():
    file_path = str(pathlib.Path(__file__).parent.absolute())
    module_path = '\\'.join(file_path.split('\\')[:-1])
    dbc_paths = glob.glob(f'{module_path}\\**\\RtRange3000.dbc', recursive=True)
    if len(dbc_paths) > 0:
        dbc_path = dbc_paths[0]
    else:
        dbc_path = None
    return dbc_path


def get_f360_mudp_resim_log_path(original_log_path: str, resim_extension: str) -> str:
    """
    Generates resim log path basing on original log path and provided resim extension.
    :param original_log_path: path to original log
    :param resim_extension: resim extension
    :return: path to resim log
    """
    path = pathlib.Path(original_log_path)
    parent_dir = path.parent
    log_name = path.stem
    return str(parent_dir / resim_extension / f'{log_name}_{resim_extension}.mudp')


def map_array_values(input_array: np.ndarray, mapping_dict: dict, default_value: any):
    """
    Return new array which contains mapped values of input_array.
    :param input_array: array which should be mapped
    :param mapping_dict: dictionary in form {input value: output value}
    :param default_value: is used if not all values from input array were mapped
    :return: mapped array which has same shape as input_array

    Example:
    input_array = np.array([2, 3, 1, 4, 900])
    mapping_dict = {1: 'cat', 2: 'mouse', 3: 'dog'}
    default_value: 'unknown_animal'
    out_array = map_array_values(input_array, mapping_dict, default_value)
    out_array = ndarray(['mouse', 'dog', 'cat', 'unknown_animal', 'unknown_animal'])
    """
    output_array = np.full(input_array.shape, default_value)
    for in_value, out_value in mapping_dict.items():
        output_array = np.where(input_array == in_value, out_value, output_array)
    return output_array


def filter_out_nan_columns(df_in: pd.DataFrame):
    '''
    Remove columns which contains only nan values from DataFrame and return it's copy.
    :param df_in: input DataFrame
    :return: reduced DataFrame
    '''
    nan_columns = [s for s in df_in if np.all(df_in[s].isna())]
    non_nans_columns = df_in.columns.difference(nan_columns)
    return df_in.loc[:, non_nans_columns]


def cast_enum_columns_to_ints(arr_in: np.ndarray):
    enum_to_int = np.vectorize(lambda x: x.value)
    enum_col_to_enum_class_map = {}
    out = np.zeros(shape=arr_in.shape, dtype=int)
    for column_index in range(arr_in.shape[1]):
        signal_vals = arr_in[:, column_index]
        out[:, column_index] = enum_to_int(signal_vals).astype(int)
        enum_col_to_enum_class_map[column_index] = signal_vals[0].__class__
    return out, enum_col_to_enum_class_map


def cast_int_columns_to_enums(arr_in: np.ndarray, enum_col_to_enum_class: dict):
    out = np.zeros(shape=arr_in.shape, dtype=object)
    for column_index in range(arr_in.shape[1]):
        signal_vals = arr_in[:, column_index]
        enum_class = enum_col_to_enum_class[column_index]
        enum_signal = np.array([*enum_class], object)[
            signal_vals.astype(int)]  # this is very fast way to cast int to enums
        out[:, column_index] = enum_signal
    return out


def get_column_indexes(df: pd.DataFrame, column_names: list):
    columns = list(df.columns)
    column_idxs = [columns.index(f) for f in column_names]
    return column_idxs


def concat_list_of_arrays_to_dataframe(list_of_arrays, columns):
    out = None
    if len(list_of_arrays) > 0:
        signals_vals = np.vstack(list_of_arrays)
        out = pd.DataFrame(data=signals_vals, columns=columns)
    return out


def get_nested_dict_keys(d: dict, prefix=()) -> Iterable[tuple]:
    """
    Yields keys of nested dictionary as path tuples, e.g. ('parsed_data', 6, 'obj').
    :param d: nested dictionary
    :param prefix: key path prefix (for recursive calls)
    :return: iterable of nested keys
    """
    for k, v in d.items():
        if isinstance(v, dict):
            yield from get_nested_dict_keys(v, prefix + (k,))
        else:
            yield prefix + (k,)


def iterate_parents_and_find_directory(root: pathlib.Path, searched_dir_name: str, max_iter: int = 3):
    output_path = None
    for iter_num, dir_path in enumerate(root.parents, start=1):
        srr_refs_list = list(dir_path.glob(searched_dir_name))
        if len(srr_refs_list) > 0:
            output_path = dir_path / searched_dir_name
            break
        if iter_num >= max_iter:
            break
    return output_path


def rms(in_array: Union[List[numbers.Real], np.ndarray]):
    """
    Return root mean square of in_array
    @param in_array:
    @return: rms
    """
    return np.sqrt(np.mean(np.square(in_array)))


def concat_dict_of_lists_of_dataframes(dict_of_lists: dict):
    """
    Function which transforms dict in form of:
    dict_of_lists = {
        'key_1': [df_1_A, df_1_B, df_1_C],
        'key_2': [df_2_A, df_2_B]
    }
    to:
    dict_of_dataframes = {
        'key_1': df_1,
        'key_2': df_2
    }
    ,where df_1 is concatenated df_1_A, df_1_B and df_1_C. Same with df_2.

    :param dict_of_lists: dictionary in form: {str: list[pd.DataFrame]}
    :return: dict of dataframes in form: {str: pd.DataFrame}
    """
    dict_of_dataframes = {}
    for key, list_of_dfs in dict_of_lists.items():
        try:
            dict_of_dataframes[key] = pd.concat(list_of_dfs).reset_index(drop=True)
        except TypeError as e:
            print(e)
            print(f"Error during concatenating {key}")
    return dict_of_dataframes


def create_pickle_path(path_file):
    """
    Function which create a path to pickle file of log

    Only the following bin files are supported:
    - TrackerOutput
    - OSIGTInput

    :param path_file: path to log file
    :return: path to the pickle file
    """
    log_extension = str(path_file).split('.')[-1]
    if log_extension == 'mudp':
        pickle_save_path = str(path_file).replace('.mudp', '_f360_mudp_extracted.pickle')
    elif log_extension == 'xtrk':
        pickle_save_path = str(path_file).replace('.xtrk', '_f360_xtrk_extracted.pickle')
    elif log_extension == 'mf4' and 'BN_FASETH' in path_file:
        pickle_save_path = str(path_file).replace('.mf4', '_f360_mf4_bmw_mid_extracted.pickle')
    elif log_extension == 'bin' and 'ENV' in path_file:
        pickle_save_path = str(path_file).replace('.bin', '_env_bin_extracted.pickle')
    elif log_extension == 'bin' and '_TrackerOutput' in path_file:
        pickle_save_path = str(path_file).replace('_TrackerOutput.bin', '_gdsr_bin_extracted.pickle')
    elif log_extension == 'bin' and '_OSIGTInput' in path_file:
        pickle_save_path = str(path_file).replace('_OSIGTInput.bin', '_dspace_bin_extracted.pickle')
    elif log_extension == 'keg':
        pickle_save_path = str(path_file).replace('.keg', '_env_keg_extracted.pickle')
    else:
        raise ValueError(f"Log file with extension: {log_extension}; is not supported")

    return pickle_save_path


def is_log_newer_than_pickle(log_path, pickle_path):
    """
    Function that check which file has a newer update
    :param file_path: absolute path to log file
    :param pickle_path: absolute path to pickle file of log above
    :return: True/False if creation time of log file is newer/older than pickle
    """
    if not pathlib.Path(pickle_path).exists():
        return False

    def get_file_time_when_last_updated(file):
        creation_time = os.path.getctime(file)
        modification_time = os.path.getmtime(file)
        if creation_time > modification_time:
            return creation_time
        else:
            return modification_time

    return get_file_time_when_last_updated(log_path) > get_file_time_when_last_updated(pickle_path)


def create_log_path(path_file):
    str_to_remove = path_file.split('\\')[-1]
    log_path = path_file.replace(str_to_remove, '')
    return log_path


def remove_suffix(file_name):
    separator = '.'
    splitted_file_name = file_name.split(separator)[:-1]
    new_file_name = separator.join(splitted_file_name)
    return new_file_name


def remove_log_extension(log_name: str):
    """
    Remove suffix from string.
    :param log_name: str
    :return: reduced str
    """
    suffix = log_name.split('.')[-1]
    suffix = '.' + suffix
    log_name = remove_suffix(log_name)
    return log_name


def timing_decorator(func):
    def inner(*args, **kwargs):
        t1 = time()
        out = func(*args, **kwargs)
        t2 = time()
        print(f"{func.__name__} call time: {t2 - t1}")
        return out

    return inner


def replace_file_extension(file_path: str, new_ext: str):
    """
    Replace given file extension with new one.
    :param file_path:
    :param new_ext:
    :return:
    """
    ext = Path(file_path).suffix
    new_ext = f'.' + new_ext.replace('.', '')  # add . if it was missing
    return str(file_path).replace(ext, new_ext)


def calc_dets_azimuth_vcs(extracted_data):
    if extracted_data.detections is not None:
        dets = extracted_data.detections.signals
        if extracted_data.sensors is not None and 'azimuth_vcs' not in dets:
            sensors = extracted_data.sensors.per_sensor
            dets_w_sensors = dets.join(sensors.set_index('sensor_id'), on='sensor_id', rsuffix='_sensor')
            dets['azimuth_vcs'] = dets_w_sensors.azimuth + dets_w_sensors.boresight_az_angle


def contiguous_regions(condition):
    """Finds contiguous True regions of the boolean array "condition". Returns
    a 2D array where the first column is the start index of the region and the
    second column is the end index."""

    # Find the indicies of changes in "condition"
    d = np.diff(condition)
    idx, = d.nonzero()

    # We need to start things after the change in "condition". Therefore,
    # we'll shift the index by 1 to the right.
    idx += 1

    if condition[0]:
        # If the start of condition is True prepend a 0
        idx = np.r_[0, idx]

    if condition[-1]:
        # If the end of condition is True, append the length of the array
        idx = np.r_[idx, condition.size]  # Edit

    # Reshape the result into two columns
    idx.shape = (-1, 2)
    return idx


def get_dec_rgb(color: str):
    """
    Get red, green, blue values from color string in DEC.
    @param color: color string in HEX format
    @return: Red, green, blue values of color in DEC
    """
    r = int(color[1:3], 16)
    g = int(color[3:5], 16)
    b = int(color[5:7], 16)
    return r, g, b


def add_alpha_to_rgb(color: str, alpha: str):
    """
    Adds alpha channel to RGB string, unless alpha is already in string.
    @param color: RGB color string in Hex
    @param alpha: aplha color string in Hex (without # or 0x)
    @return: RGBA color string
    """
    if len(color) < 8:
        return color + alpha
    else:
        return color
