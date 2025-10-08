# coding=utf-8
"""
Main Data Set interface
"""
from abc import ABC, abstractmethod
from copy import deepcopy
from enum import Enum
from typing import List
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.SignalDescription import GeneralSignals
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription
from aspe.utilities.interpolation import (
    interpolate_angular_values,
    interpolate_bools,
    interpolate_enums,
    interpolate_floats,
    interpolate_ints,
    interpolate_lists,
)
from aspe.utilities.SupportingFunctions import (
    concat_list_of_arrays_to_dataframe,
    filter_out_nan_columns,
    get_column_indexes,
    map_array_values,
)


class IDataSet(ABC):
    """
    Data Set interface class

    Signals description:

    Set of signals for linkage between DataSets records (in DataSet.signals) within single DataSetContainer:
        scan_index -  unique identifier for data coming from the same package of data. Several records within DataSet
                      can have same scan_index.
                      Each DataSet within single DataSetContainer has the same scan base (identified by scan_index)
        timestamp - time identifier for each single record  - there may be different timestamps for
                    records with the same scan index.
                    Each DataSet within single DataSetContainer has the same time base (identified by timestamp).

    Entity orientation signals:
        slot_id - location in parsed data for array-structured data (id of column) - zero-based
        unique_id - unique identifier for entity (lifetime) within all records in DataSet - zero-based
    """

    def __init__(self):
        super(IDataSet, self).__init__()
        signal_names = [GeneralSignals.scan_index,
                        GeneralSignals.timestamp,
                        GeneralSignals.unique_id]

        self.angular_signals = []
        self.signals_info = pd.DataFrame()
        self.signals = pd.DataFrame()
        self.update_signals_definition(signal_names)
        self.base_signal_names = self.signals_info['signature'].to_list()
        self.raw_signals = None

    def get_signal_names_by_type(self, signal_type: type) -> List[str]:
        mask = self.signals_info.loc[:, 'dtype'] == signal_type
        signal_names_series = self.signals_info.loc[mask, 'signature']
        return list(signal_names_series.to_numpy())

    def update_signals_definition(self, list_of_signal: List[SignalDescription]):
        list_of_series = [s_d.to_series() for s_d in list_of_signal]
        signals_info_local = pd.DataFrame(list_of_series)
        dtypes = {row['signature']: row['dtype'] for _, row in signals_info_local.iterrows()}
        signals_local = pd.DataFrame(columns=signals_info_local['signature'].to_list())
        signals_local = signals_local.astype(dtypes)
        self.signals_info = self.signals_info.append(signals_info_local)
        self.signals = pd.concat([self.signals, signals_local], axis=1, sort=False)

    def interpolate_values(self, new_timestamp_vals, new_scan_index_vals, timestamp_signal_name='timestamp'):

        def interp_single_obj(obj: np.ndarray, signal_idxs: list, list_to_aggregate: list, obj_timestamp: np.ndarray,
                              ts_to_interp: np.ndarray, interp_fun: object):
            if len(signal_idxs) > 0:
                interp_vals = interp_fun(obj[:, signal_idxs], obj_timestamp, ts_to_interp)
                list_to_aggregate.append(interp_vals)

        signals_filtered = filter_out_nan_columns(self.signals)

        float_sig_names = signals_filtered.columns.intersection(self.get_signal_names_by_type(np.float32), sort=False)
        float_sig_names = float_sig_names.append(
            signals_filtered.columns.intersection(self.get_signal_names_by_type(np.float64), sort=False))
        float_sig_names = list(set(float_sig_names) - set(self.angular_signals))

        int_sig_names = (signals_filtered.columns.intersection(self.get_signal_names_by_type(int), sort=False))
        int_sig_names = int_sig_names.append(
            signals_filtered.columns.intersection(self.get_signal_names_by_type(np.int8), sort=False))
        int_sig_names = int_sig_names.append(
            signals_filtered.columns.intersection(self.get_signal_names_by_type(np.int16), sort=False))
        int_sig_names = int_sig_names.append(
            signals_filtered.columns.intersection(self.get_signal_names_by_type(np.int32), sort=False))
        int_sig_names = int_sig_names.append(
            signals_filtered.columns.intersection(self.get_signal_names_by_type(np.int64), sort=False))

        enum_sig_names = signals_filtered.columns.intersection(self.get_signal_names_by_type('category'), sort=False)
        bool_sig_names = signals_filtered.columns.intersection(self.get_signal_names_by_type(bool), sort=False)
        list_sig_names = signals_filtered.columns.intersection(self.get_signal_names_by_type(list), sort=False)

        float_signals_idxs = get_column_indexes(signals_filtered, float_sig_names)
        int_signals_idxs = get_column_indexes(signals_filtered, int_sig_names)
        enum_signals_idxs = get_column_indexes(signals_filtered, enum_sig_names)
        bool_signals_idxs = get_column_indexes(signals_filtered, bool_sig_names)
        list_signals_idxs = get_column_indexes(signals_filtered, list_sig_names)
        angular_signals_idxs = get_column_indexes(signals_filtered, self.angular_signals)

        floats2join, ints2join, enums2join, bools2join, lists2join, angular2join = [], [], [], [], [], []
        for unique_id, obj in signals_filtered.groupby(by='unique_id'):
            try:
                obj_timestamp = obj.loc[:, timestamp_signal_name].to_numpy()
            except KeyError:
                warn(f'Timestamp signal: \'{timestamp_signal_name}\' within {self.__class__} is missing or not filled. '
                     f'Interpolation cannot be done. Skipping this data set.')
                return
            obj_ts_born, obj_ts_died = obj_timestamp[0], obj_timestamp[-1]
            ts_within_obj = new_timestamp_vals[(obj_ts_born < new_timestamp_vals) & (new_timestamp_vals < obj_ts_died)]

            obj_numpy = obj.to_numpy()
            # perform interpolations for all data types and aggregate results to corresponding list
            interp_single_obj(obj_numpy, float_signals_idxs, floats2join, obj_timestamp, ts_within_obj,
                              interpolate_floats)
            interp_single_obj(obj_numpy, int_signals_idxs, ints2join, obj_timestamp, ts_within_obj, interpolate_ints)
            interp_single_obj(obj_numpy, enum_signals_idxs, enums2join, obj_timestamp, ts_within_obj, interpolate_enums)
            interp_single_obj(obj_numpy, bool_signals_idxs, bools2join, obj_timestamp, ts_within_obj, interpolate_bools)
            interp_single_obj(obj_numpy, list_signals_idxs, lists2join, obj_timestamp, ts_within_obj, interpolate_lists)
            interp_single_obj(obj_numpy, angular_signals_idxs, angular2join, obj_timestamp, ts_within_obj,
                              interpolate_angular_values)

        # stack results arrays and convert it to dataframes
        zipped = zip([floats2join, ints2join, enums2join, bools2join, lists2join, angular2join],
                     [float_sig_names, int_sig_names, enum_sig_names, bool_sig_names, list_sig_names,
                      self.angular_signals])
        dfs_to_concat = [concat_list_of_arrays_to_dataframe(list_of_arrays=vals, columns=names) for vals, names in
                         zipped]
        signals_out = pd.concat([arr for arr in dfs_to_concat if arr is not None], axis=1)

        ts_to_scan_index_map = dict(zip(list(new_timestamp_vals), list(new_scan_index_vals)))
        out_si = map_array_values(signals_out.loc[:, timestamp_signal_name].to_numpy(), ts_to_scan_index_map, 0)
        signals_out.loc[:, 'scan_index'] = out_si
        missing_columns = list(self.signals.columns.difference(signals_out.columns))
        signals_out = pd.concat([signals_out, pd.DataFrame(columns=missing_columns, index=signals_out.index)], axis=1)
        self.signals = signals_out

    @abstractmethod
    def get_base_name(self):
        """
        Get base name of data set
        :return: str: name of the data set
        """
        pass

    def base_signals_update(self, input_df: pd.DataFrame):
        """
        Modifies object with self.signals from other DataFrame
        :param input_df:
        :type input_df: pd.DataFrame
        :return:
        """
        self.signals[self.base_signal_names] = input_df[self.base_signal_names]
        return self

    def to_dict(self):
        """
        Converts the dataset into a generic Python dictionary.
        :return: Dictionary of data set and all nested objects
        """
        return deepcopy(self.__dict__)

    def transfer_cs(self, coordinate_sys: str, dist_rear_axle_to_front_bumper: float):
        if coordinate_sys == 'ISO_VRACS':
            self._transfer_to_iso_vracs(dist_rear_axle_to_front_bumper)
        elif coordinate_sys == 'VCS':
            self._transfer_to_vcs(dist_rear_axle_to_front_bumper)
        elif coordinate_sys == 'PANCS':
            self._transfer_to_pancs(dist_rear_axle_to_front_bumper)
        else:
            raise ValueError(f'Coordinate system: {coordinate_sys} not known - transformation cannot be done')

    def _transfer_to_iso_vracs(self, dist_rear_axle_to_front_bumper: float):
        pass

    def _transfer_to_vcs(self, dist_rear_axle_to_front_bumper: float):
        pass

    def _transfer_to_pancs(self, dist_rear_axle_to_front_bumper):
        pass

    def update_signals_dtypes(self):
        dtype_dict = dict(zip(self.signals_info.signature, self.signals_info.dtype))
        for signal, sig_dtype in dtype_dict.items():
            if signal in self.signals and (not self.signals[signal].isnull().values.any()):
                self.signals[signal] = self.signals[signal].astype(sig_dtype)
