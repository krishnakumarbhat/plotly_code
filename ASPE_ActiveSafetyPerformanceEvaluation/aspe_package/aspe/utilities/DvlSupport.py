# coding=utf-8
"""
Set of functions for dvl extraction support
"""
from warnings import warn

import numpy as np
import pandas as pd


def general_extract_utc(target_dataframe: pd.DataFrame, datetime_mapping_dict: dict) -> pd.Series:
    """
    Function mainly dedicated for RTDvlExtractor!
    Wrapped function to not repeating code
    :param target_dataframe: particualr target dtaframe
    :param datetime_mapping_dict: dictionary of datetime mapping class
    :return: pandas series of utc. returning Nan when some signals are missing
    """
    mapping_utc_signals = \
        [
            datetime_mapping_dict['century'][-1],
            datetime_mapping_dict['year'][-1],
            datetime_mapping_dict['month'][-1],
            datetime_mapping_dict['day'][-1],
            datetime_mapping_dict['hour'][-1],
            datetime_mapping_dict['minute'][-1],
            datetime_mapping_dict['seconds'][-1],
            datetime_mapping_dict['ms'][-1],
        ]
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
        target_time = target_dataframe.loc[:, mapping_utc_signals]
        target_time.columns = utc_signals_names

        target_time.loc[:, 'century'] *= 100  # century in Rt are represent as for e.g. 20 instead of 2000
        target_time.loc[:, 'ms'] *= 1000  # despite of 'ms' name Rt provides it in seconds (for e.g. 0.19)
        target_time.loc[:, 'year'] += target_time['century']

        target_time = target_time.drop(columns=['century'])
        target_utc_time = pd.to_datetime(target_time)
    except KeyError:
        target_utc_time = np.nan
        warn('cant find some signals for utc parsing... skipping this part')

    return target_utc_time


def get_rt_extraction_possibility_flag(parsed_data, target_str_name: str) -> bool:
    f_is_extraction_possible = False
    if target_str_name in parsed_data:
        f_is_extraction_possible = True
    return f_is_extraction_possible


def extract_message_sets(parsed_data: dict, msg_sets_config: dict):
    """
    Group DVL parsed data messages to message sets using msg_sets_config dict. In this dictionary specify which frame ID
    is master ID and which are slaves ID for specific data set. Each master ID occurrence will be interpreted as new
    time instance and frames with slaves ID which occur before next master ID will be put to this time instance.
    Output is dictionary with N data frames containing signals data for specific data set.

    :param msg_sets_config: dictionary containing information how to group message sets. Example of how this dict should
    be look like:

    {
    4: {  # DVL channel number
        data_set_1: {   # first message sets group - output dataframe will be called data_set_1
                    "master_frame_id": "300",  # master frame id - in hex!
                    "slave_frame_id": ["600", "601", "603", "604", "607", "608"] # slave frame ids list - in hex!
                    }
        data_set_2: {...}}

    :type msg_sets_config: dict
    :param parsed_data: output from dvl parser
    :type parsed_data: dict
    :return: dict of data frames with grouped message sets
    """
    def group_message_sets(channel_data, master_id, slave_ids):
        """
        Method that parsing single message ts by given parsing function
        :return: grouped message sets
        """
        list_main_timestamps = sorted(channel_data[master_id].keys())
        messages = {}
        signals = set()
        for slave_id in slave_ids:
            if slave_id in channel_data:
                messages.update(channel_data[slave_id])
        for key, message in messages.items():
            signals.update(message.keys())
        all_ts = np.sort(np.array(list(messages.keys())))
        data_len = len(list_main_timestamps)
        main_timestamps = np.array(list_main_timestamps)
        out_dict = {'timestamp': main_timestamps,
                    'f_iscomplete': np.full(data_len, False)}
        for signal in signals:
            out_dict[signal] = np.full(data_len, np.nan)
        for main_ts_idx, main_ts in enumerate(main_timestamps):
            if main_ts_idx < len(main_timestamps) - 1:
                next_ts = main_timestamps[main_ts_idx + 1]
                tstmps_within_range = all_ts[(main_ts <= all_ts) & (all_ts < next_ts)]
            else:  # handle end of the vector
                tstmps_within_range = all_ts[main_ts <= all_ts]
            extracted_signals = []
            for curr_ts in tstmps_within_range:
                curr_message = messages[curr_ts]
                for signal, value in curr_message.items():
                    out_dict[signal][main_ts_idx] = value
                    extracted_signals.append(signal)
            if set(extracted_signals) == signals:
                out_dict['f_iscomplete'][main_ts_idx] = True
        return pd.DataFrame(out_dict)

    output = {}
    for channel, decoded_chanel in parsed_data.items():
        if channel in msg_sets_config:
            msg_sets_channel_config = msg_sets_config[channel]
            for msg_set_name, msg_set_conf in msg_sets_channel_config.items():
                mst_id = int(msg_set_conf['master_frame_id'], 16)
                slv_ids = [int(s_id, 16) for s_id in msg_set_conf['slave_frame_id']]  # list of hex to list of ints
                if mst_id in decoded_chanel:
                    msg_set_group = group_message_sets(decoded_chanel, mst_id, slv_ids)
                    output[msg_set_name] = msg_set_group
        else:
            warn(f"Channel {channel} not found in parsed DVL data, check message sets configuration and set "
                 "proper channel number for used data. Returning empty dict")
    return output