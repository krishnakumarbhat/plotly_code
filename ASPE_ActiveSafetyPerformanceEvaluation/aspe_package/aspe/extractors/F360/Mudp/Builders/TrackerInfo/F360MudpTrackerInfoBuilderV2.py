# coding=utf-8
"""
F360 MUDP tracker info builder (version 2)
"""
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.F360.Enums.f360_objects import F360ObjectStatus
from aspe.extractors.F360.Mudp.Builders.TrackerInfo.IF360MudpTrackerInfoBuilder import IF360MudpTrackerInfoBuilder
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals
from aspe.utilities.SupportingFunctions import get_nested_array, is_1d_array, values_are_same


class F360MudpTrackerInfoBuilderV2(IF360MudpTrackerInfoBuilder):
    """
    supports host extraction for stream definitions:
    ch: 5
    - strdef_src035_str005_ver014
    - strdef_src035_str005_ver015
    - strdef_src035_str005_ver016
    - strdef_src035_str005_ver017
    - strdef_src035_str005_ver018
    - strdef_src035_str005_ver019 (not tested)

    ch: 7
    - strdef_src035_str007_ver002.txt,
    - strdef_src035_str007_ver003.txt
    - strdef_src035_str007_ver004.txt
    - strdef_src035_str007_ver005.txt (not tested)
    - strdef_src035_str007_ver006.txt (not tested)
    - strdef_src035_str007_ver007.txt (not tested)

    """
    required_stream_definitions = [['strdef_src035_str005_ver014', 'strdef_src035_str005_ver015',
                                    'strdef_src035_str005_ver016', 'strdef_src035_str005_ver017',
                                    'strdef_src035_str005_ver018', 'strdef_src035_str005_ver019',
                                   'strdef_src035_str005_ver020', 'strdef_src035_str005_ver021'],

                                   ['strdef_src035_str007_ver002', 'strdef_src035_str007_ver003',
                                    'strdef_src035_str007_ver004', 'strdef_src035_str007_ver005',
                                    'strdef_src035_str007_ver006', 'strdef_src035_str007_ver007']]
    main_stream = 7
    objects_stream = 5
    object_key = [objects_stream, 'obj']
    signal_mapper = {
        # ASPE signature                            F360 signature
        GeneralSignals.scan_index.signature:       'tracker_index',
    }
    properties_mapper = {
        # ASPE signature    F360 signature
        'sw_version_minor': 'sw_version_minor',
        'sw_version_major': 'sw_version_major',
        'sw_version_patch': 'sw_version_patch',
        'build_id':         'sw_version_buildID',
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)

    def build(self):
        """
        Main tracker info build function
        :return: tracker info dataset
        """
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        self._convert_dtypes()
        return self.data_set

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        raw_signals_dict = self._parsed_data['parsed_data'][self.main_stream]
        for name, values in raw_signals_dict.items():
            if is_1d_array(values):
                self._raw_signals[name] = values

        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        """
        Extract signals which can not be strictly mapped using Mapper object.
        :return:
        """
        self._extract_timestamp()
        self._extract_utc_timestamp()

    def _extract_timestamp(self):
        """
        Extracts timestamp by shifting tracker execution timestamp by mean time since measure.
        :return:
        """
        timestamp_us = self._raw_signals['timestamp_us']
        tracker_execution_timestamp = timestamp_us * 1e-6  # get timestamp in seconds
        self.data_set.signals['execution_timestamp'] = tracker_execution_timestamp

        objects_data = get_nested_array(self._parsed_data['parsed_data'], self.object_key)
        if objects_data is None:
            warn('Objects stream not found, timestamp will not be shifted by mean time since measure')
            self.data_set.signals['timestamp'] = tracker_execution_timestamp
            return

        objects_time_since_measure = objects_data['time_since_measurement']
        updated_mask = objects_data['reducedStatus'] == F360ObjectStatus.UPDATED.value
        time_since_measure_for_updated = objects_time_since_measure[updated_mask]
        mean_time_since_measure = time_since_measure_for_updated.mean()

        self.data_set.signals['timestamp'] = tracker_execution_timestamp - mean_time_since_measure

    def _extract_properties(self):
        """
        Extracts properties.
        :return:
        """
        properties = {}
        for aspe_signature, raw_signature in self.properties_mapper.items():
            try:
                property_values = self._raw_signals[raw_signature]
            except KeyError:
                warn(f'Signal {raw_signature} not found, property {aspe_signature} will not been extracted')
                continue

            if not values_are_same(property_values.values):
                warn(f'Signal {raw_signature} has different values, property {aspe_signature} will not been extracted')
                continue

            properties[aspe_signature] = property_values[0]

        if 'build_id' in properties:
            build_id = int(properties['build_id'])
        else:
            warn(f'build_id property not found, build ID will not be extracted')
            build_id = None
        properties['build_id'] = build_id

        if {'sw_version_major', 'sw_version_minor', 'sw_version_patch'}.issubset(set(properties.keys())):
            sw_ver_major = str(properties['sw_version_major'])
            sw_ver_minor = str(properties['sw_version_minor'])
            sw_ver_patch = str(properties['sw_version_patch'])
            separator = '.'
            sw_version = separator.join((sw_ver_major, sw_ver_minor, sw_ver_patch))
        else:
            warn(f'sw_version property will not be extracted')
            sw_version = None
        properties['sw_version'] = sw_version

        self.data_set.__dict__.update(properties)

    def _extract_utc_timestamp(self):
        if 99 in self._parsed_data['parsed_data']:
            timing_stream = self._parsed_data['parsed_data'][99]
            utc_time = timing_stream['utc_timestamp_ns'] * 1e-9  # [s] in unix time
            tracker_exec_time = timing_stream['sensor_timestamp_us'] * 1e-6  # [s]

            utc_time_df = pd.DataFrame(data={'utc_timestamp': utc_time, 'tracker_exec_timestamp': tracker_exec_time})
            self.data_set.signals = self.data_set.signals.join(utc_time_df.set_index('tracker_exec_timestamp'),
                                                               on='execution_timestamp')
