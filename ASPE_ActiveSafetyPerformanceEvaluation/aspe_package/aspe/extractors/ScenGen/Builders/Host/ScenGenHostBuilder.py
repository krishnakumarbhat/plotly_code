"""
Scenario Generator host builder
"""
import warnings

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces import IHost
from aspe.extractors.Interfaces.SignalDescription import PhysicalSignals
from aspe.extractors.ScenGen.Builders.ScenGenBuilder import ScenGenBuilder
from aspe.extractors.ScenGen.DataSets.ScenGenHost import ScenGenHost


class ScenGenHostBuilder(ScenGenBuilder):
    signal_mapper = {
        # ASPE signature             #Scenario Generator signature
        IHost.raw_speed:            'vv',
        PhysicalSignals.yaw_rate:   'yy',
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super(ScenGenHostBuilder, self).__init__(parsed_data)

        self.data_set = ScenGenHost()
        self._raw_signals = pd.DataFrame()
        self._parsed_data_shape = None

        self._f_extract_raw_signals = f_extract_raw_signals

    def build(self):
        """Main extraction function."""

        self._set_parsed_data_shape()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        self._convert_dtypes()

        return self.data_set

    def _set_parsed_data_shape(self):
        """Sets parsed data shape based on xpos_wcs signal."""
        host = self._parsed_data['vehicle_data']
        self._parsed_data_shape = host['xpos_wcs'].shape

    def _extract_raw_signals(self):
        """
        Extract raw signals.

        Mysterious raw signals description:
            - dt - time interval [s]
            - vv - velocity [m/s]
            - yy - yaw rate [rad/s]
            - hh - heading [deg]
            - ss - traveled way [m]
        """
        host_data = self._parsed_data['vehicle_data']

        for name, value in host_data.items():
            if isinstance(value, np.ndarray):
                self._raw_signals[name] = value
            elif isinstance(value, dict):
                continue

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        self._extract_scan_index()
        self._extract_timestamp()
        self._extract_unique_id()
        self._extract_position()
        self._extract_abs_raw_speed()

    def _extract_scan_index(self):
        """
        Extract scan index based on log length and detections data shape.
        Scan index is not present in Scenario Generator logs, so it needs to be arranged.
        """
        scan_index_vector = np.arange(self._parsed_data_shape[0])
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

    def _extract_unique_id(self):
        """
        Extract unique ids.
        It is assumed that there is only one column (one host)
        """
        self.data_set.signals['unique_id'] = np.arange(self._parsed_data_shape[0])

    def _extract_position(self):
        """
        Extract host position.
        Host position in VCS is fixed and equals 0.
        """
        self.data_set.signals['position_x'] = 0
        self.data_set.signals['position_y'] = 0

    def _extract_abs_raw_speed(self):
        self.data_set.signals['abs_raw_speed'] = self.data_set.signals['raw_speed'].abs()

    def _extract_properties(self):
        self.data_set.coordinate_system = 'VCS'  # center of front bumper
        self.data_set.bounding_box_dimensions_x = self._parsed_data['vehicle_data']['common']['host_length']
        self.data_set.bounding_box_dimensions_y = np.nan  # no signal available
        self.data_set.bounding_box_orientation = 0.0  # assumed to be aligned with VCS X axis
        self.data_set.bounding_box_refpoint_para_offset_ratio = 1.0  # center of front bumper
        self.data_set.bounding_box_refpoint_orh_offset_ratio = 0.5  # center of front bumper

        dist_of_rear_axle = self._parsed_data['vehicle_data']['common']['rear_axle_x_pos_vcs']
        self.data_set.dist_of_rear_axle_to_front_bumper = dist_of_rear_axle
