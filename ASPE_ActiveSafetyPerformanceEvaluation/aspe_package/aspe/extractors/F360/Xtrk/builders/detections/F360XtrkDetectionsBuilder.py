# coding=utf-8
"""
F360 XTRK detections builder
"""
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets import F360RadarDetections
from aspe.extractors.F360.Xtrk.builders.F360XtrkBuilder import F360XtrkBuilder
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals


class F360XtrkDetectionsBuilder(F360XtrkBuilder):
    """
    supports objects extraction for xtrk.
    """

    signal_mapper = {
        # ASPE signature                          # f360 signature - if it's a list builder ckecks if any of
                                                  # listed signals is in f360 signals list - multiple version handling
        PhysicalSignals.position_x:               ['vcs_position.longitudinal', 'vcs_position.x'],
        PhysicalSignals.position_y:               ['vcs_position.lateral', 'vcs_position.y'],
        F360RadarDetections.assigned_obj_id:      'object_track_id',
        F360RadarDetections.cluster_id:           'cluster_id',
        F360RadarDetections.range_rate_comp:      'range_rate_compensated',
        F360RadarDetections.range_rate_dealiased: 'range_rate_dealiased',
        F360RadarDetections.motion_status:        ['motion_status', 'processed.motion_status'],
        F360RadarDetections.wheel_spin_status:    'wheel_spin_type',
        F360RadarDetections.f_dealiased:          'f_dealiased',
        GeneralSignals.sensor_id:                 ['sensor_id', 'raw.sensor_id'],
        PhysicalSignals.range_rate:               ['range_rate', 'raw.range_rate'],
        F360RadarDetections.raw_det_id:           ['det_id', 'raw.det_id'],
        PhysicalSignals.range:                    ['range', 'raw.range'],
        PhysicalSignals.azimuth:                  ['azimuth', 'raw.azimuth'],
    }

    # Signal mapping, plain list corresponds to nested location in structure, list of lists can be used to handle
    # multiple possible signal locations
    auxiliary_signals = {
        'scan_index': ['trkrInfo', 'cnt_loops'],
        'sensor_timestamps': [['sensors', 'timestamp_us'], ['sensors', 'variable.timestamp_us']],
        'sensor_look_id': [['sensors', 'look_id'], ['sensors', 'variable.look_id']],
        'sensor_boresight_az_angle': [['sensors', 'mounting_position.vcs_boresight_azimuth_angle'],
                                      ['sensors', 'constant.mounting_position.vcs_boresight_azimuth_angle']],
        'std_range': [['raw_dets', 'std_range'], ['raw_dets', 'raw.std_range']],
        'std_range_rate': [['raw_dets', 'std_range_rate'], ['raw_dets', 'raw.std_range_rate']],
        'std_azimuth': [['raw_dets', 'std_azimuth'], ['raw_dets', 'raw.std_azimuth']],
        'det_id': [['raw_dets', 'det_id'], ['raw_dets', 'raw.det_id'], ['det_id']],
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self._raw_signals = pd.DataFrame()
        self.data_set = F360RadarDetections.F360RadarDetections()
        self.auxiliary_raw_signals = self._get_auxiliary_raw_signals_mapping(self.auxiliary_signals)
        self._f_extract_raw_signals = f_extract_raw_signals

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
        slot_id = self._get_nested_array_using_possible_signatures(self._parsed_data, self.auxiliary_signals['det_id'])
        self._mask = np.argwhere(slot_id != 0)

    def _set_parsed_data_shape(self):
        slot_id = self._get_nested_array_using_possible_signatures(self._parsed_data, self.auxiliary_signals['det_id'])
        self._parsed_data_shape = slot_id.shape

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        raw_dets = self._parsed_data['raw_dets']
        det_props = self._parsed_data['det_props']
        raw_signals = {**raw_dets, **det_props}

        for name, values in raw_signals.items():
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
        rows = self._mask[:, 0]
        self._extract_scan_index()
        self.data_set.signals['slot_id'] = self._mask[:, 1]
        self.data_set.signals['unique_id'] = np.arange(0, self.data_set.signals.shape[0])
        sensor_index = self.data_set.signals['sensor_id'] - 1
        try:
            self.data_set.signals['timestamp'] = self._extract_signal_by_sensor_idx(rows, sensor_index,
                                                                                'sensor_timestamps')
        except KeyError:
            warn(f'Signal sensor_timestamps not found, timestamp will not be extracted')
        try:
            self.data_set.signals['sensor_look_id'] = self._extract_signal_by_sensor_idx(rows, sensor_index,
                                                                                     'sensor_look_id')
        except KeyError:
            warn(f'Signal sensor_look_id not found, sensor_look_id will not be extracted')

        signal_list = ['range', 'azimuth', 'range_rate']
        for signature in signal_list:
            try:
                std_signal = \
                    self._get_nested_array_using_possible_signatures(self._raw_signals,
                                                                     self.auxiliary_raw_signals['std_' + signature])
                self.data_set.signals[signature+'_variance'] = std_signal ** 2
            except KeyError:
                warn(f"Signal {'raw.std_'+signature} not found, {signature+'_variance'} will not been extracted")

    def _extract_scan_index(self):
        """
        Extract scan index data
        :return:
        """
        rows = self._mask[:, 0]
        scan_idx_vector = self._get_nested_array_using_possible_signatures(self._parsed_data,
                                                                           self.auxiliary_signals['scan_index'])
        if scan_idx_vector is None:
            warn('Tracker info stream or tracker_index signal not found, scan_index will start from 0')
            scan_idx_vector = np.arange(self._parsed_data_shape[0])
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
        aux_signal = self._get_nested_array_using_possible_signatures(self._parsed_data,
                                                                      self.auxiliary_signals[aux_signal_name])
        return aux_signal[rows, sensor_idx]

    def _extract_properties(self):
        self.data_set.max_slots_number = self._parsed_data_shape[1]
        self.data_set.coordinate_system = 'VCS'
