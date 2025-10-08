# coding=utf-8
"""
F360 MUDP Trailer builder (version 1)
"""
from warnings import warn

import numpy as np

from aspe.extractors.F360.Mudp.Builders.Host.IF360MudpTrailerBuilder import IF360MudpTrailerBuilder
from aspe.utilities.SupportingFunctions import get_nested_array, is_1d_array


class F360MudpTrailerBuilderV1(IF360MudpTrailerBuilder):
    """
    supports trailer extraction for stream definitions:
        - strdef_src035_str086_ver002
    """
    required_stream_definitions = [['strdef_src035_str086_ver002']]
    main_stream = 86
    signal_mapper = {
        # ASPE signature                                F360 signature
    }
    auxiliary_signals = {
        'scan_index': [main_stream, 'stream_ref_index'],
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)
        self._parsed_data_shape = None

    def build(self):
        """ Main extraction function. """
        self._set_parsed_data_shape()

        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._convert_dtypes()
        return self.data_set

    def _set_parsed_data_shape(self):
        """
        Function to find out shape of input (parsed) data

        :return: shape of parsed host data
        """
        trailer_length = get_nested_array(self._parsed_data['parsed_data'], [self.main_stream, 'trailer_length'])
        self._parsed_data_shape = trailer_length.shape

    def _extract_raw_signals(self):
        """ Extracts all raw signals from parsed data into raw signals dataframe. """
        raw_signals_parsed_dict = self._parsed_data['parsed_data'][self.main_stream]

        for raw_signal_name, raw_signal_values in raw_signals_parsed_dict.items():
            if  is_1d_array(raw_signal_values.shape):
                self._raw_signals[raw_signal_name] = raw_signal_values

        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        """
        Handling common signals extraction for which direct mapping is not possible
        """
        self.data_set.signals['unique_id'] = np.full(self._parsed_data_shape, 0)
        self._extract_scan_index()
        self._extract_trailer_coupling_point()
        self._extract_bbox_properties()

    def _extract_scan_index(self):
        """ Extract scan index. """
        scan_index = get_nested_array(self._parsed_data['parsed_data'], self.auxiliary_signals['scan_index'])

        if scan_index is None:
            warn('Tracker info stream or tracker_index signal not found, scan_index will start from 0')
            scan_index = np.arange(self._parsed_data_shape[0])

        self.data_set.signals['scan_index'] = scan_index

    def _extract_trailer_coupling_point(self):
        """ Extracts trailer coupling point. """
        host_length = 5.0
        self.data_set.signals['position_x'] = -host_length
        self.data_set.signals['position_y'] = 0.0

    def _extract_bbox_properties(self):
        """ Extracts bounding box properties. """
        self.data_set.signals['bounding_box_dimensions_x'] = self._raw_signals['trailer_length']
        self.data_set.signals['bounding_box_dimensions_y'] = self._raw_signals['trailer_width']
        self.data_set.signals['bounding_box_orientation'] = self._raw_signals['trailer_angle']
