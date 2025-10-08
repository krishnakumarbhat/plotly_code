# coding=utf-8
"""
F360 MUDP detections builder (version 15)
"""
from warnings import warn

import numpy as np

from aspe.extractors.F360.DataSets import F360RadarDetections
from aspe.extractors.F360.Mudp.Builders.Detections.IF360MudpDetectionsBuilder import IF360MudpDetectionsBuilder
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.extractors.Specification.radars import DefaultRadar
from aspe.utilities.MathFunctions import cart2pol, vcs2scs
from aspe.utilities.SupportingFunctions import get_nested_array


class F360MudpDetectionsBuilderV15(IF360MudpDetectionsBuilder):
    """
    supports objects extraction for stream definitions:
    ch: 71
        - strdef_src035_str071_ver001
    """
    required_stream_definitions = [['strdef_src035_str071_ver001', 'strdef_src035_str071_ver002'],

                                   ['strdef_src035_str072_ver001', 'strdef_src035_str072_ver002']]
    main_stream = 71
    sensors_stream = 72
    detections_key = [main_stream]
    sensors_key = [sensors_stream]
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
    auxiliary_signals = { # Should probably be from sensor stream but data dimensions does not agree
        'scan_index': [main_stream, 'stream_ref_index'],
        'sensor_id': [sensors_stream, 'id'],
        'sensor_timestamps': [sensors_stream, 'data_timestamp_us'],
        'sensor_x_vcs': [sensors_stream, 'vcs_position_lateral'],
        'sensor_y_vcs': [sensors_stream, 'vcs_position_longitudinal'],
        'sensor_boresight_az_angle': [sensors_stream, 'vcs_boresight_azimuth_angle'],
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)

    def build(self):
        """
        Main extraction function.
        :return:
        """
        self._extract_valid_sensors_idxs()
        self._calc_mask()
        self._set_parsed_data_shape()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        self._convert_dtypes()
        return self.data_set

    def _extract_valid_sensors_idxs(self):
        """
        Extract valid sensor index and indicate sensor configuration
        """

        sensors_valid_arr = get_nested_array(self._parsed_data['parsed_data'], self.auxiliary_signals['sensor_id'])

        self._f_single_sensor_config = True if sensors_valid_arr.ndim == 1 else False
        self._valid_sensors_ids = np.where(np.sum(sensors_valid_arr, axis=0) != 0)[0]

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

        x_scs, y_scs = vcs2scs(self.data_set.raw_signals['vcs_x'],
                               self.data_set.raw_signals['vcs_y'],
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

        # Azimuth expected to be in raw_signals for calc_dets_azimuth_vcs function later
        self.data_set.raw_signals['azimuth'] = self.data_set.signals['azimuth']
        self.data_set.raw_signals['range'] = self.data_set.signals['range']

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

        return aux_signal[rows, sensor_idx] if not self._f_single_sensor_config else aux_signal[rows]

    def _extract_properties(self):
        self.data_set.max_slots_number = self._parsed_data_shape[1]
        self.data_set.coordinate_system = 'VCS'
