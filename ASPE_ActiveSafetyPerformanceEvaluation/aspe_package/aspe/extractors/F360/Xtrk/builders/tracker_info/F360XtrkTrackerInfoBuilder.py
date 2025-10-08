# trk
# coding=utf-8
"""
F360 XTRK tracker info builder
"""
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.F360TrackerInfo import F360TrackerInfo
from aspe.extractors.F360.Xtrk.builders.F360XtrkBuilder import F360XtrkBuilder
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals
from aspe.utilities.SupportingFunctions import get_nested_array, is_1d_array, values_are_same


class F360XtrkTrackerInfoBuilder(F360XtrkBuilder):
    """
    supports tracker info extraction from xtrk files
    """

    signal_mapper = {
        # ASPE signature              F360 signature
        GeneralSignals.scan_index:    'cnt_loops',
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self.data_set = F360TrackerInfo()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = pd.DataFrame()

    def build(self):
        """
        Main tracker info build function
        :return: tracker info dataset
        """
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._convert_dtypes()
        return self.data_set

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        raw_signals_dict = self._parsed_data['trkrInfo']
        for name, values in raw_signals_dict.items():
            if is_1d_array(values):
                self._raw_signals[name] = values.reshape(-1)

        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        """
        Extract signals which can not be strictly mapped using Mapper object.
        :return:
        """
        self._extract_timestamp()

    def _extract_timestamp(self):
        """
        Extracts timestamp by shifting tracker execution timestamp by mean time since measure.
        :return:
        """
        try:
            timestamp_us = self._raw_signals['time_us']
            self.data_set.signals['execution_timestamp'] = (timestamp_us * 1e-6).astype(np.float32)
        except KeyError:
            warn('Signal time_us is not found, execution_timestamp will not be extracted')

        try:
            obj_list_timestamp_us = self._raw_signals['object_list_timestamp']
            self.data_set.signals['timestamp'] = obj_list_timestamp_us * 1e-6
        except KeyError:
            warn('Signal object_list_timestamp is not found, timestamp will not be extracted')