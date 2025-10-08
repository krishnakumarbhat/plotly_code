"""
Scenario Generator detections builder
"""
import warnings

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.extractors.ScenGen.Builders.ScenGenBuilder import ScenGenBuilder
from aspe.extractors.ScenGen.DataSets.ScenGenDetections import ScenGenDetections


class ScenGenDetectionsBuilder(ScenGenBuilder):
    signal_mapper = {
        # ASPE signature                            # Scenario Generator signature
        PhysicalSignals.position_x:                 'x_vcs',
        PhysicalSignals.position_y:                 'y_vcs',
        PhysicalSignals.azimuth:                    'azim',
        PhysicalSignals.range:                      'range',
        PhysicalSignals.range_rate:                 'rngrate',
        PhysicalSignals.amplitude:                  'ampl',
        GeneralSignals.look_id:                     'sensor_short_long_look',
    }

    auxiliary_signals = {
        'reflection_angle': 'refl_angle',
        'mount_location': 'sensor_mount_loc',
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super(ScenGenDetectionsBuilder, self).__init__(parsed_data)

        self._f_extract_raw_signals = f_extract_raw_signals

        self.data_set = ScenGenDetections()
        self._raw_signals = pd.DataFrame()
        self._parsed_data_shape = None

    def build(self) -> ScenGenDetections:
        """Main extraction function."""

        self._set_parsed_data_shape()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        self._convert_dtypes()

        return self.data_set

    def _set_parsed_data_shape(self):
        """Set parsed data shape basen on x_vcs detection signal."""
        dets = self._parsed_data['detections']
        self._parsed_data_shape = dets['x_vcs'].shape

    def _extract_raw_signals(self):
        dets = self._parsed_data['detections']

        for name, values in dets.items():
            if values.shape == self._parsed_data_shape:
                self._raw_signals[name] = values.flatten()

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        self._extract_auxiliary()
        self._extract_scan_index()
        self._extract_timestamp()
        self._extract_sensor_id()
        self._drop_empty_detections()
        self._extract_unique_id()

    def _extract_auxiliary(self):
        for aspe_name, secenario_name in self.auxiliary_signals.items():
            self.data_set.signals[aspe_name] = self._raw_signals[secenario_name]

    def _extract_scan_index(self):
        """
        Extract scan index based on log length and detections data shape.
        Scan index is not present in Scenario Generator logs, so it needs to be arranged.
        """
        scan_index = np.arange(self._parsed_data_shape[0])
        scan_index_vector = np.repeat(scan_index, self._parsed_data_shape[1])
        self.data_set.signals['scan_index'] = scan_index_vector

    def _extract_timestamp(self):
        """
        Extract timestamp based on scan indexes.
        Timestamp is not present in Scenario Generator logs, so it needs to be arranged.
        To do that, 'dt' value given in host data is used, if 'dt' is missing default value is used.
        """
        host_data = self._parsed_data['vehicle_data']
        if 'dt' in host_data.keys():
            dt = host_data['dt']
        else:
            dt = 0.05  # set default value
            warnings.warn("'dt' signal is missing in host data, default value set.")
        timestamp_vector = self.data_set.signals['scan_index'] * dt
        self.data_set.signals['timestamp'] = timestamp_vector

    def _extract_sensor_id(self):
        """
        Extraction of sensor ids.
        Sensor id signals is extracted by mapping sensor mount location, which is given in proper order.
        """
        val, idx = np.unique(self._raw_signals['sensor_mount_loc'], return_index=True)
        mount_loc_values = val[idx.argsort()]

        for idx, el in enumerate(mount_loc_values):
            mask = self.data_set.signals['mount_location'] == el
            self.data_set.signals.loc[mask, 'sensor_id'] = idx

    def _drop_empty_detections(self):
        """
        Drop empty detections from data set signals, based on f_valid signal.
        In Scenario Generator logs detections data structure got fixed shape. If detection is not valid it is filled
        with NaN value. This function drops not valid/NaN detections before assignment of unique id.
        """
        f_valid = self._raw_signals['f_valid'].apply(lambda row: True if row == 1 else False)
        self.data_set.signals = self.data_set.signals[True == f_valid]
        self.data_set.signals.reset_index(inplace=True, drop='index')

    def _extract_unique_id(self):
        """Extract unique ids, based on df indexes."""
        self.data_set.signals['unique_id'] = self.data_set.signals.index

    def _extract_properties(self):
        self.data_set.max_slots_number = self._parsed_data['vehicle_data']['common']['sensor_num_rays'].sum()
        self.data_set.coordinate_system = 'VCS'
