# coding=utf-8
"""
F360 MUDP tracker info builder (version 1)
"""
from warnings import warn

import numpy as np

from aspe.extractors.F360.Enums.f360_objects import F360ObjectStatus
from aspe.extractors.F360.Mudp.Builders.TrackerInfo.IF360MudpTrackerInfoBuilder import IF360MudpTrackerInfoBuilder
from aspe.utilities.SupportingFunctions import get_nested_array, is_1d_array, values_are_same


class F360MudpTrackerInfoBuilderV1(IF360MudpTrackerInfoBuilder):
    """
    supports host extraction for stream definitions:
    ch: 5 - strdef_src035_str005_ver012.txt
    ch: 7 - strdef_src035_str007_ver001.txt
    """
    required_stream_definitions = {'strdef_src035_str005_ver012', 'strdef_src035_str007_ver001'}
    main_stream = 7
    objects_stream = 5
    object_key = [objects_stream, 'obj']
    signal_mapper = {
        # ASPE signature    F360 signature
        'scan_index':       'tracker_index',
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

    def _extract_mappable_signals(self):
        """
        Extract signals which can be strictly mapped using Mapper object.
        :return:
        """
        for aspe_signature, raw_signature in self.signal_mapper.items():
            try:
                self.data_set.signals[aspe_signature] = self._raw_signals[raw_signature]
            except KeyError:
                warn(f'Signal {raw_signature} not found, {aspe_signature} will not been extracted')

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
        tracker_execution_timestamp = self._raw_signals['timestamp']
        self.data_set.signals['execution_timestamp'] = tracker_execution_timestamp

        objects_data = get_nested_array(self._parsed_data['parsed_data'], self.object_key)
        if objects_data is None:
            warn('Objects stream not found, timestamp will not be shifted by mean time since measure')
            self.data_set.signals['timestamp'] = tracker_execution_timestamp
            return

        objects_timestamp = objects_data['timestamp']
        updated_mask = objects_data['reducedStatus'] == F360ObjectStatus.UPDATED.value
        objects_count = objects_timestamp.shape[1]
        trk_exec_timestamps_mat = np.tile(tracker_execution_timestamp,
                                          (objects_count, 1)).T  # work with parsers >= 1.3.2
        mean_time_since_measure = (trk_exec_timestamps_mat[updated_mask] - objects_timestamp[updated_mask]).mean()
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
