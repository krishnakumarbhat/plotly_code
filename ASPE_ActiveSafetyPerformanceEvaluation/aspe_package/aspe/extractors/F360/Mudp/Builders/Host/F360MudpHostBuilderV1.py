# coding=utf-8
"""
F360 MUDP host builder (version 1)
"""
from warnings import warn

import numpy as np

from aspe.extractors.F360.Mudp.Builders.Host.IF360MudpHostBuilder import IF360MudpHostBuilder
from aspe.utilities.SupportingFunctions import get_nested_array, get_unique_value_with_most_counts


class F360MudpHostBuilderV1(IF360MudpHostBuilder):
    """
    supports host extraction for stream definitions:
    ch: 4 - strdef_src035_str004_ver001.txt
    ch: 7 - strdef_src035_str007_ver001.txt
    """
    required_stream_definitions = {'strdef_src035_str004_ver001', 'strdef_src035_str007_ver001'}
    main_stream = 4
    tracker_info_stream = 7
    signal_mapper = {
        # ASPE signature        F360 signature
        'raw_speed':            'raw_speed',
        'abs_raw_speed':        'abs_raw_speed',
        'yaw_rate':             'yawrate',
        'acceleration_otg_x':   'vcs_long_acceleration',
        'acceleration_otg_y':   'vcs_lat_acceleration',
    }
    auxiliary_signals = {
        'scan_index': [tracker_info_stream, 'tracker_index'],
        'timestamp': [tracker_info_stream, 'timestamp'],
    }
    properties = {
        'dist_of_rear_axle_to_front_bumper': [main_stream, 'dist_rear_axle_to_vcs'],
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)
        self._parsed_data_shape = None

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

        return self.data_set

    def _set_parsed_data_shape(self):
        """
        Function to find out shape of input (parsed) data
        :return: shape of parsed host data
        """
        raw_speed = get_nested_array(self._parsed_data['parsed_data'], [self.main_stream, 'raw_speed'])
        self._parsed_data_shape = raw_speed.shape

    def _extract_raw_signals(self):
        """
        Extracts all raw signals from parsed data into raw signals dataframe.
        :return:
        """
        raw_signals_parsed_dict = self._parsed_data['parsed_data'][self.main_stream]

        for raw_signal_name, raw_signal_values in raw_signals_parsed_dict.items():
            if len(raw_signal_values.shape) == 1:
                self._raw_signals[raw_signal_name] = raw_signal_values

        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_mappable_signals(self):
        """
        Extract signals which can be strictly mapped using Mapper object
        :return:
        """
        for aspe_signature, raw_signature in self.signal_mapper.items():
            try:
                self.data_set.signals[aspe_signature] = self._raw_signals[raw_signature]
            except KeyError:
                warn(f'Signal {raw_signature} not found, {aspe_signature} will not been extracted')

    def _extract_non_mappable_signals(self):
        """
        Handling common signals extraction for which direct mapping is not possible
        :return:
        """
        # It is assumed that there is only one column (one host)
        self.data_set.signals['unique_id'] = np.full(self._parsed_data_shape, 0)
        self.data_set.signals['slot_id'] = 0

        self._extract_scan_index()
        self._extract_timestamp()
        self._extract_position()
        self._extract_velocity()

    def _extract_scan_index(self):
        scan_index = get_nested_array(self._parsed_data['parsed_data'], self.auxiliary_signals['scan_index'])

        if scan_index is None:
            warn('Tracker info stream or tracker_index signal not found, scan_index will start from 0')
            scan_index = np.arange(self._parsed_data_shape[0])

        self.data_set.signals['scan_index'] = scan_index

    def _extract_timestamp(self):
        timestamp = get_nested_array(self._parsed_data['parsed_data'], self.auxiliary_signals['timestamp'])

        if timestamp is None:
            warn('Tracker info stream or timestamp signal not found, timestamp will not be extracted')
            return

        self.data_set.signals['timestamp'] = timestamp

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
        otg_speed = self._raw_signals['vcs_speed']
        otg_heading = self._raw_signals['vcs_sideslip']

        self.data_set.signals['velocity_otg_x'] = np.cos(otg_heading) * otg_speed
        self.data_set.signals['velocity_otg_y'] = np.sin(otg_heading) * otg_speed

    def _extract_properties(self):
        # TODO: Dimensions should be available in vehicle stream
        self.data_set.coordinate_system = 'VCS'  # center of front bumper
        self.data_set.bounding_box_dimensions_x = np.nan  # no signal available
        self.data_set.bounding_box_dimensions_y = np.nan  # no signal available
        self.data_set.bounding_box_orientation = 0.0  # assumed to be aligned with VCS X axis
        self.data_set.bounding_box_refpoint_para_offset_ratio = 1.0  # center of front bumper
        self.data_set.bounding_box_refpoint_orh_offset_ratio = 0.5  # center of front bumper

        dist_of_rear_axle_values = get_nested_array(self._parsed_data['parsed_data'],
                                                    self.properties['dist_of_rear_axle_to_front_bumper'])

        self.data_set.dist_of_rear_axle_to_front_bumper = \
            get_unique_value_with_most_counts(dist_of_rear_axle_values)
