# coding=utf-8
"""
F360 MUDP detections builder (version 14)
"""
from warnings import warn

import numpy as np

from aspe.extractors.F360.DataSets import F360RadarDetections
from aspe.extractors.F360.Mudp.Builders.Detections.IF360MudpDetectionsBuilder import IF360MudpDetectionsBuilder
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.extractors.Specification.radars import DefaultRadar
from aspe.utilities.MathFunctions import cart2pol, vcs2scs
from aspe.utilities.SupportingFunctions import get_nested_array


class F360MudpDetectionsBuilderV14(IF360MudpDetectionsBuilder):
    """
    supports objects extraction for stream definitions:
    ch: 3
        - strdef_src035_str003_ver014
        - strdef_src035_str003_ver015
        - strdef_src035_str003_ver016
        - strdef_src035_str003_ver017 (not tested)
    """
    required_stream_definitions = [['strdef_src035_str003_ver014', 'strdef_src035_str003_ver015',
                                    'strdef_src035_str003_ver016', 'strdef_src035_str003_ver017',
                                    'strdef_src035_str003_ver018']]
    main_stream = 3
    tracker_info_stream = 7
    detections_key = [main_stream, 'detsLog']
    signal_mapper = {
        # ASPE signature                          F360 signature
        GeneralSignals.sensor_id:                 'sensorID',
        PhysicalSignals.range_rate:               'rngrate_dealiased',
        PhysicalSignals.position_x:               'vcs_x',
        PhysicalSignals.position_y:               'vcs_y',
        F360RadarDetections.raw_det_id:           'raw_det_id',
        F360RadarDetections.assigned_obj_id:      'objTrkID',
        F360RadarDetections.cluster_id:           'clusterID',
        F360RadarDetections.range_rate_comp:      'rngrate_comp',
        F360RadarDetections.range_rate_dealiased: 'rngrate_dealiased',
        F360RadarDetections.motion_status:        'motion_status',
        F360RadarDetections.wheel_spin_status:    'wheel_spin',
        F360RadarDetections.f_dealiased:          'f_dealiased',
    }
    auxiliary_signals = {
        'scan_index': [main_stream, 'stream_ref_index'],
        'sensor_timestamps': [main_stream, 'sensorInfoLog', 'timestamp_us'],
        'sensor_look_id': [main_stream, 'sensorInfoLog', 'look_id'],
        'sensor_boresight_az_angle': [main_stream, 'sensorInfoLog', 'vcs_boresight_az_angle'],
        'sensor_x_vcs': [main_stream, 'sensorInfoLog', 'vcs_long_posn'],
        'sensor_y_vcs': [main_stream, 'sensorInfoLog', 'vcs_lat_posn'],
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)

    def build(self):
        """
        Main extraction function.
        :return:
        """
        self._calc_mask()
        self._set_parsed_data_shape()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        self._convert_dtypes()
        return self.data_set

    def _calc_mask(self):
        """
        Get information about indexes of detection data. Mask is tuple of 2 ndarrays - 1st array contains row indecies
        and2nd contains column indecies
        :return:
        """
        slot_id = get_nested_array(self._parsed_data['parsed_data'], self.detections_key + ['raw_det_id'])
        self._mask = np.argwhere(slot_id != 0)

    def _set_parsed_data_shape(self):
        slot_id = get_nested_array(self._parsed_data['parsed_data'], self.detections_key + ['raw_det_id'])
        self._parsed_data_shape = slot_id.shape

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        parsed_raw_signals_dict = get_nested_array(self._parsed_data['parsed_data'], self.detections_key)

        for name, values in parsed_raw_signals_dict.items():
            if len(values.shape) == 2:
                converted_signal = values[self._mask[:, 0], self._mask[:, 1]]
                self._raw_signals[name] = converted_signal

        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        """
        Extract signals which cannot be strictly mapped using mapper object. It contains 'scan_idx', 'slot_id',
        'uniqe_id', 'timestamp', 'look_id', 'range', 'azimuth'
        :return:
        """
        # TODO Handle situation when empty array from _extract_signal_by_sensor_idx is returned, improve overall
        #  robustness
        rows = self._mask[:, 0]
        self._extract_scan_index()
        self.data_set.signals['slot_id'] = self._mask[:, 1]
        self.data_set.signals['unique_id'] = np.arange(0, self.data_set.signals.shape[0])
        sensor_index = self.data_set.signals['sensor_id'] - 1
        self.data_set.signals['timestamp'] = self._extract_signal_by_sensor_idx(rows, sensor_index,
                                                                                'sensor_timestamps') * 1e-6
        self.data_set.signals['sensor_look_id'] = self._extract_signal_by_sensor_idx(rows, sensor_index,
                                                                                     'sensor_look_id')
        x_scs, y_scs = vcs2scs(self.data_set.signals['position_x'],
                               self.data_set.signals['position_y'],
                               self._extract_signal_by_sensor_idx(rows, sensor_index,
                                                                  'sensor_x_vcs'),
                               self._extract_signal_by_sensor_idx(rows, sensor_index,
                                                                  'sensor_y_vcs'),
                               self._extract_signal_by_sensor_idx(rows, sensor_index,
                                                                  'sensor_boresight_az_angle'))

        self.data_set.signals['range'], self.data_set.signals['azimuth'] = cart2pol(x_scs, y_scs)
        self.data_set.signals['range_variance'] = DefaultRadar.range_std ** 2
        self.data_set.signals['azimuth_variance'] = DefaultRadar.azimuth_std ** 2
        self.data_set.signals['range_rate_variance'] = DefaultRadar.range_rate_std ** 2

    def _extract_scan_index(self):
        """
        Extract scan index data
        :return:
        """
        rows = self._mask[:, 0]
        scan_idx_vector = get_nested_array(self._parsed_data['parsed_data'], self.auxiliary_signals['scan_index'])
        scan_index_mapping_class = np.vectorize(lambda row: scan_idx_vector[row])
        detections_scan_idnexes = scan_index_mapping_class(rows)
        self.data_set.signals['scan_index'] = detections_scan_idnexes

    def _extract_signal_by_sensor_idx(self, rows, sensor_idx, aux_signal_name):
        """
        Extract detection data which is stored in sensor info data. Relevant signal
        is same for every sensor's detection - so information should be taken from sensor data and written to
        right detection slot
        :param rows: array of row indecies - contains infromation about in which row of parsed data detection is stored
        :param sensor_idx: array of sensor_idx- contains infromation about detection sensor_idx
        :param aux_signal_name: signal which is being extracted from sensor_info struct
        :return:
        """
        aux_signal = get_nested_array(self._parsed_data['parsed_data'],
                                      self.auxiliary_signals[aux_signal_name])
        return aux_signal[rows, sensor_idx]

    def _extract_properties(self):
        self.data_set.max_slots_number = self._parsed_data_shape[1]
        self.data_set.coordinate_system = 'VCS'
