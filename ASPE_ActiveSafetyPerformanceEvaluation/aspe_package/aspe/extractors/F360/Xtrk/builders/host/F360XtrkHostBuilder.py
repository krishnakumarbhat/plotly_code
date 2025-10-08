# coding=utf-8
"""
F360 Xtrk host builder
"""
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.F360.DataSets.F360Host import F360Host
from aspe.extractors.F360.Xtrk.builders.F360XtrkBuilder import F360XtrkBuilder
from aspe.extractors.Interfaces import IHost
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.utilities.SupportingFunctions import get_nested_array, get_unique_value_with_most_counts, is_1d_array


class F360XtrkHostBuilder(F360XtrkBuilder):
    """
    supports host extraction from .xtrk file
    """
    signal_mapper = {
        # ASPE signature                       F360 signature
        GeneralSignals.timestamp:             'timestamp_s',
        IHost.raw_speed:                      'raw_speed',
        PhysicalSignals.yaw_rate:             'raw_yaw_rate_rad',
        PhysicalSignals.acceleration_otg_x:   'vcs_long_acceleration',
        PhysicalSignals.acceleration_otg_y:   'vcs_lat_acceleration',
    }
    auxiliary_signals = {
        'scan_index': ['trkrInfo', 'cnt_loops'],
    }
    properties = {
        'dist_of_rear_axle_to_front_bumper': ['host_calib', 'dist_rear_axle_to_vcs_m'],
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)
        self._parsed_data_shape = None
        self._f_extract_raw_signals = f_extract_raw_signals
        self.data_set = F360Host()
        self._raw_signals = pd.DataFrame()

    def build(self):
        """
        Main extraction function.
        :return:
        """
        self._set_parsed_data_shape()

        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        self._convert_dtypes()
        return self.data_set

    def _set_parsed_data_shape(self):
        """
        Function to find out shape of input (parsed) data
        :return: shape of parsed host data
        """
        raw_speed = get_nested_array(self._parsed_data, ['host_raw', 'raw_speed'])
        self._parsed_data_shape = raw_speed.shape[0]

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        host_calib = self._parsed_data['host_calib']  # maybe should be added as properties?
        host_raw = self._parsed_data['host_raw']
        host = self._parsed_data['host']
        host_props = self._parsed_data['host_props']

        raw_signals = {**host_calib, **host_raw, **host, **host_props}

        for raw_signal_name, raw_signal_values in raw_signals.items():
            if is_1d_array(raw_signal_values):
                self._raw_signals[raw_signal_name] = raw_signal_values.reshape(-1)
        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        """
        Handling common signals extraction for which direct mapping is not possible
        :return:
        """
        # It is assumed that there is only one column (one host)
        self.data_set.signals['unique_id'] = np.full(self._parsed_data_shape, 0)
        self.data_set.signals['slot_id'] = 0

        self._extract_scan_index()
        self._extract_position()
        self._extract_velocity()
        self._extract_abs_raw_speed()

    def _extract_scan_index(self):
        scan_index = get_nested_array(self._parsed_data, self.auxiliary_signals['scan_index'])

        if scan_index is None:
            warn('Tracker info stream or tracker_index signal not found, scan_index will start from 0')
            scan_index = np.arange(self._parsed_data_shape[0])

        self.data_set.signals['scan_index'] = scan_index

    def _extract_position(self):
        """
        Extracts position signals.
        :return:
        """
        # Position in VCS is always fixed and equal to zero
        self.data_set.signals['position_x'] = 0
        self.data_set.signals['position_y'] = 0

    def _extract_velocity(self):
        """
        Calculates velocity signals from speed and heading.
        :return:
        """
        try:
            otg_speed = self._raw_signals['vcs_speed']
            otg_heading = self._raw_signals['vcs_sideslip']

            self.data_set.signals['velocity_otg_x'] = np.cos(otg_heading) * otg_speed
            self.data_set.signals['velocity_otg_y'] = np.sin(otg_heading) * otg_speed
        except KeyError:
            warn(f'Signal vcs_speed or vcs_sideslip not found, velocity_otg_x and velocity_otg_y will not be extracted')


    def _extract_abs_raw_speed(self):
        try:
            self.data_set.signals['abs_raw_speed'] = self.data_set.signals['raw_speed'].abs()
        except KeyError:
            warn(f'Signal raw_speed not found, velocity_otg_x and abs_raw_speed will not be extracted')

    def _extract_properties(self):
        # TODO: Dimensions should be available in vehicle stream
        self.data_set.coordinate_system = 'VCS'  # center of front bumper
        self.data_set.bounding_box_dimensions_x = np.nan  # no signal available
        self.data_set.bounding_box_dimensions_y = np.nan  # no signal available
        self.data_set.bounding_box_orientation = 0.0  # assumed to be aligned with VCS X axis
        self.data_set.bounding_box_refpoint_para_offset_ratio = 1.0  # center of front bumper
        self.data_set.bounding_box_refpoint_orh_offset_ratio = 0.5  # center of front bumper

        dist_of_rear_axle_values = get_nested_array(self._parsed_data,
                                                    self.properties['dist_of_rear_axle_to_front_bumper'])

        self.data_set.dist_of_rear_axle_to_front_bumper = \
            get_unique_value_with_most_counts(dist_of_rear_axle_values)
