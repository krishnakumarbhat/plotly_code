# coding=utf-8
"""
F360 MUDP static environment builder
"""

from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.F360StaticEnvPolynomials import F360StaticEnvPolynomials
from aspe.extractors.F360.DataSets.SignalDescription import F360CoreSignals
from aspe.extractors.Mudp.IMudpBuilder import IMudpBuilder
from aspe.utilities.objects_lifespan_status_based_creator import ObjectLifespanStatusBasedCreator
from aspe.utilities.SupportingFunctions import get_nested_array


class F360MudpStaticEnvironmentBuilderV2(IMudpBuilder):
    """
    supports static environment extraction for stream definitions:
    ch: 19
        - strdef_src035_str019_ver002
    """
    required_stream_definitions = [['strdef_src035_str019_ver002']]
    stat_env_struct_name = 'static_env_polys'

    main_stream = 19
    signal_mapper = {
        # ASPE signature                        F360 signature
        # common data sets section
        # unified object interface section
        # coordinate system defined in properties
        F360CoreSignals.polynomial_p0:          'p0',
        F360CoreSignals.polynomial_p1:          'p1',
        F360CoreSignals.polynomial_p2:          'p2',
        F360CoreSignals.x_min_limit:            'lower_limit',
        F360CoreSignals.x_max_limit:            'upper_limit',
        F360CoreSignals.stat_env_type:          'poly_type',
        F360CoreSignals.status:                 'status',
        F360CoreSignals.confidence_level:       'confidence',
    }
    auxiliary_signals = {
        'timestamp_us': [main_stream, 'timestamp_us'],
        'scan_index':   [main_stream, 'tracker_index'],
        'age':          [main_stream, 'static_env_polys', 'age'],
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self._raw_signals = pd.DataFrame()
        self.data_set = F360StaticEnvPolynomials()
        self._f_extract_raw_signals = f_extract_raw_signals

    def build(self):
        """
        Main extraction function.
        :return:
        """
        self._set_parsed_data_shape()
        self._create_lifespan_mask()

        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._convert_dtypes()
        return self.data_set

    def _set_parsed_data_shape(self):
        """
        Sets _parsed_data_shape property.
        :return:
        """
        self._parsed_data_shape = get_nested_array(self._parsed_data['parsed_data'],
                                                   self.auxiliary_signals['age']).shape

    def _create_lifespan_mask(self):
        """
        Function which creates lifespan mask - DataFrame which 3 columns - row - column - obj_id. Each row is single
        object sample within single time instance.
        :return:
        """
        age = get_nested_array(self._parsed_data['parsed_data'], self.auxiliary_signals['age'])
        prev_age = np.roll(age, 1, axis=0)
        prev_age[0, :] = 0  # just fill first row after roll operation with invalid value

        prev_age_is_0 = prev_age == 0
        curr_age_is_non_0 = age != 0
        stat_env_born = prev_age_is_0 & curr_age_is_non_0

        new_status, exist_status, invalid_status = 1, 0, -1
        status_arr = np.full(shape=age.shape, fill_value=invalid_status)
        status_arr[curr_age_is_non_0] = exist_status
        status_arr[stat_env_born] = new_status
        self._lifespan_mask = ObjectLifespanStatusBasedCreator.create_lifespan(status_arr,
                                                                               new_status,
                                                                               invalid_status)

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        lifespan_mask = (self._lifespan_mask['row'], self._lifespan_mask['column'])
        raw_signals_parsed_dict = self._parsed_data['parsed_data'][self.main_stream][self.stat_env_struct_name]

        columns_count = self._parsed_data_shape[1]
        for raw_signal_name in raw_signals_parsed_dict.keys():
            signal_values = raw_signals_parsed_dict[raw_signal_name]

            if len(signal_values.shape) == 2:
                if signal_values.shape[1] == columns_count:
                    converted_signal = signal_values[lifespan_mask]
                    self._raw_signals[raw_signal_name] = converted_signal
                else:
                    warn('Invalid number of columns in signal values, signal will not be extracted')
            elif len(signal_values.shape) > 2:
                if signal_values.shape[1] == columns_count:
                    converted_signal = signal_values[lifespan_mask]
                    self._raw_signals[raw_signal_name] = list(converted_signal)
                else:
                    warn('Invalid number of columns in signal values, signal will not be extracted')

        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        """
        Extract signals which can not be strictly mapped using Mapper object.
        :return:
        """
        self.data_set.signals['unique_id'] = self._lifespan_mask['unique_obj_id']
        self.data_set.signals['slot_id'] = self._lifespan_mask['column']
        self._extract_scan_index()
        self._extract_timestamp()

    def _extract_scan_index(self):
        """
        Extract scan_index signal from tracker_info structure.
        :return:
        """
        scan_idx_vector = get_nested_array(self._parsed_data['parsed_data'],
                                           self.auxiliary_signals['scan_index'])
        if scan_idx_vector is None:
            warn('Tracker info stream or tracker_index signal not found, scan_index will start from 0')
            scan_idx_vector = np.arange(self._parsed_data_shape[0])

        lifespan_mask = (self._lifespan_mask['row'])
        self.data_set.signals['scan_index'] = scan_idx_vector[lifespan_mask]

    def _extract_timestamp(self):
        """
        Auxiliary implementation of timestamp extraction, currently only for F360MudpInternalObjectsBuilderV14.
        For F360MudpInternalObjectsBuilderV12 timestamp is extracted as a mappable signal.
        Timestamp is returned in [s] (converted from [us] to [s]).
        :return:
        """
        object_list_timestamp_us = get_nested_array(self._parsed_data['parsed_data'],
                                                    self.auxiliary_signals['timestamp_us'])
        if object_list_timestamp_us is None:
            warn('Objectlist timestamp signal not found, timestamp will not be extracted')
            return

        object_list_timestamp = object_list_timestamp_us * 1e-6
        lifespan_mask = self._lifespan_mask['row']
        object_list_timestamp_signal = object_list_timestamp[lifespan_mask]
        self.data_set.signals['timestamp'] = object_list_timestamp_signal
