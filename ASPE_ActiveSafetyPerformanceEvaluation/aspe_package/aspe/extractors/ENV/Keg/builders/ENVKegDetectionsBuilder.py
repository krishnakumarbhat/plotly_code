from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.ENV.DataSets import ENVDetections
from aspe.extractors.ENV.Keg.builders.ENVKegBuilder import ENVKegBuilder
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.utilities.SupportingFunctions import get_nested_array


class ENVKegDetectionsBuilder(ENVKegBuilder):
    """
    Supports objects extraction for keg.
    """
    signal_mapper = {
        # ASPE signature                            F360 signature
        PhysicalSignals.position_x:                 'vcs_position.x',
        PhysicalSignals.position_y:                 'vcs_position.y',
        GeneralSignals.sensor_id:                   'raw.sensor_id',
        PhysicalSignals.range_rate:                 'raw.range_rate',
        PhysicalSignals.range:                      'raw.range',
        PhysicalSignals.azimuth:                    'raw.azimuth',
        PhysicalSignals.amplitude:                  'raw.rcs',
        PhysicalSignals.elevation:                  'raw.elevation',
        ENVDetections.f_nd_target:                  'raw.f_nd_target',
        ENVDetections.f_host_veh_clutter:           'raw.f_host_veh_clutter',
        ENVDetections.f_bistatic:                   'raw.f_bistatic',
        ENVDetections.f_super_res:                  'raw.f_super_res',
        ENVDetections.raw_det_id:                   'raw.det_id',
        ENVDetections.motion_status:                'processed.motion_status',
        ENVDetections.f_dealiased:                  'processed.f_dealiased',
        ENVDetections.range_rate_comp:              'processed.range_rate_compensated',
    }

    # auxilary_signals by convention contains mapping for _parsed_signals, not _raw_signals
    auxiliary_signals = {
        'scan_index': ['trkrInfo', 'cnt_loops'],
        'sensor_timestamps': ['RADAR_PARAMS', 'sensors', 'variable.timestamp_us'],
        'sensor_look_id': ['RADAR_PARAMS', 'sensors', 'variable.look_id'],
    }

    def __init__(self, parsed_data, timestamps, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self._parsed_data = parsed_data
        self._timestamps = timestamps
        self._raw_signals = pd.DataFrame()
        self.data_set = ENVDetections.ENVDetections() # TODO FZE-400 consider F360RadarDetections instead
        self._f_extract_raw_signals = f_extract_raw_signals

    def build(self):
        """
        Main extraction function.
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
        Get information about indexes of detection data. Mask is a tuple of 2 ndarrays - 1st array contains row indices
        and 2nd contains column indices
        """
        slot_id = get_nested_array(self._parsed_data, ['DETECTIONS', 'dets_raw', 'raw.det_id'])
        self._mask = np.argwhere(slot_id != 0)

    def _set_parsed_data_shape(self):
        slot_id = get_nested_array(self._parsed_data, ['DETECTIONS', 'dets_raw', 'raw.det_id'])
        self._parsed_data_shape = slot_id.shape

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        """
        raw_dets = self._parsed_data['DETECTIONS']['dets_raw']
        det_props = self._parsed_data['DETECTIONS']['dets_prop']
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
        'unique_id', 'timestamp', 'look_id', 'range', 'azimuth'
        """
        rows = self._mask[:, 0]
        self._extract_scan_index()
        self.data_set.signals['slot_id'] = self._mask[:, 1]
        self.data_set.signals['unique_id'] = np.arange(0, self.data_set.signals.shape[0])
        sensor_index = self.data_set.signals['sensor_id'] - 1
        try:
            self.data_set.signals['timestamp'] = self._extract_signal_by_sensor_idx(rows,
                                                                                    sensor_index,
                                                                                    'sensor_timestamps')
        except (KeyError, TypeError):
            # TODO:  FZE-400 Type Error is hotfix for missing sensors data
            warn(f'Signal sensor_timestamps not found, timestamp will not be extracted.')
            timestamp_mapping_class = np.vectorize(lambda row: self._timestamps[row])
            detections_timestamp = timestamp_mapping_class(rows)
            self.data_set.signals['timestamp'] = detections_timestamp
            warn(f'Timestamp filled based on metadata timestamps.')
        try:
            self.data_set.signals['sensor_look_id'] = self._extract_signal_by_sensor_idx(rows,
                                                                                         sensor_index,
                                                                                         'sensor_look_id')
        except (KeyError, TypeError):  # TODO: FZE-400 TypeError is hotfix for missing sensors data
            warn(f'Signal sensor_look_id not found, sensor_look_id will not be extracted')

        signal_list = ['range', 'azimuth', 'range_rate']
        for signal_signature in signal_list:
            try:
                self.data_set.signals[signal_signature + '_variance'] = (
                        self._raw_signals['raw.std_' + signal_signature] ** 2)
            except KeyError:
                warn(f"Signal {'raw.std_' + signal_signature} not found,"
                     f"{signal_signature + '_variance'} will not be extracted")

    def _extract_scan_index(self):
        """
        Extract scan index data
        """
        rows = self._mask[:, 0]
        scan_idx_vector = get_nested_array(self._parsed_data, self.auxiliary_signals['scan_index'])
        if scan_idx_vector is None:
            warn('Tracker info stream or tracker_index signal not found, scan_index will start from 0')
            scan_idx_vector = np.arange(self._parsed_data_shape[0])
        scan_index_mapping_class = np.vectorize(lambda row: scan_idx_vector[row])
        detections_scan_indexes = scan_index_mapping_class(rows)
        self.data_set.signals['scan_index'] = detections_scan_indexes

    def _extract_signal_by_sensor_idx(self, rows, sensor_idx, aux_signal_name):
        """
        Extract detection data which is stored in sensor info data.

        Relevant signal is same for every sensor's detection - so information should be taken from sensor data and
        written to right detection slot

        :param rows: array of row indices - contains information about in which row of parsed data detection is stored
        :param sensor_idx: array of sensor_idx- contains information about detection sensor_idx
        :param aux_signal_name: signal which is being extracted from sensor_info struct
        """
        aux_signal = get_nested_array(self._parsed_data, self.auxiliary_signals[aux_signal_name])
        return aux_signal[rows, sensor_idx]

    def _extract_properties(self):
        self.data_set.max_slots_number = self._parsed_data_shape[1]
        self.data_set.coordinate_system = 'VCS'
