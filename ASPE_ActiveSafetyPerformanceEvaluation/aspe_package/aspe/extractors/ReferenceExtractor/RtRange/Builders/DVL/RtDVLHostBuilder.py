# coding=utf-8
"""
Rt-Range DVL Host builder
"""
from warnings import warn

import numpy as np

from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtHost import RtHost
from aspe.extractors.Specification.reference import RT3002Spec
from aspe.utilities.DvlSupport import general_extract_utc, get_rt_extraction_possibility_flag
from aspe.utilities.MathFunctions import normalize_angle_vector


class RtDVLHostBuilder(IBuilder):
    """
    Rt-Range DVL Host builder class
    """
    signal_mapper = {
        # ASPE signature    RT-Range signature
        'f_iscomplete':     'f_iscomplete',
        'position_y':       'PosLat',
        'position_x':       'PosLon',
        'velocity_otg_y':   'VelLateral',
        'velocity_otg_x':   'VelForward',
        'raw_speed':        'Speed2D',
    }
    datetime_sigs = {
        'century':          ['Host', 'TimeCentury'],
        'year':             ['Host', 'TimeYear'],
        'month':            ['Host', 'TimeMonth'],
        'day':              ['Host', 'TimeDay'],
        'hour':             ['Host', 'TimeHour'],
        'minute':           ['Host', 'TimeMinute'],
        'seconds':          ['Host', 'TimeSecond'],
        'ms':               ['Host', 'TimeHSecond'],
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False, f_completed_messages_only=True):
        super().__init__(parsed_data)
        self.data_set = RtHost()
        self.f_host_extraction_available = get_rt_extraction_possibility_flag(self._parsed_data, 'Host')
        self._f_extract_raw_signals = f_extract_raw_signals
        self._f_completed_messages_only = f_completed_messages_only
        self._raw_signals = None

    def build(self):
        """
        Main build method.
        :return: RT Host data set
        """
        if self.f_host_extraction_available:
            self._extract_raw_signals()
            self._extract_mappable_signals()
            self._extract_non_mappable_signals()
            self._extract_properties()

            if self._f_completed_messages_only:
                self._remove_incomplete_rows()

        return self.data_set

    def _extract_raw_signals(self):
        """
        Extracts raw signals from parsed data.
        :return:
        """
        self._raw_signals = self._parsed_data['Host']

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _remove_incomplete_rows(self):
        """
        Removing nan values where f_iscomplete is set to False.
        :return:
        """
        if self.data_set.signals['f_iscomplete'].empty:
            pass  # dataframe will already be clear
        else:
            self.data_set.signals = self.data_set.signals[self.data_set.signals['f_iscomplete']]

    def _extract_mappable_signals(self):
        """
        Extracts mappable signals from raw signals.
        :return:
        """
        for aspe_signature, raw_signature in self.signal_mapper.items():
            try:
                self.data_set.signals[aspe_signature] = self._raw_signals[raw_signature]
            except KeyError:
                warn(f'Signal {raw_signature} not found, {aspe_signature} will not been extracted')

    def _extract_non_mappable_signals(self):
        """
        Extracts signals that cannot be directly mapped.
        :return:
        """
        self.data_set.signals['id'] = 0
        self.data_set.signals['slot_id'] = 0
        self.data_set.signals['unique_id'] = 0

        self._extract_single_object_scan_index()
        self._extract_utc()
        self._extract_wcs_bounding_box_orientation()
        self._extract_yaw_rate()
        self._extract_timestamp()
        self._extract_variances()

    def _extract_single_object_scan_index(self):
        """
        Generating scan indexes timeline from 1 to n and adding as scan_index column to signals
        :return:
        """
        self.data_set.signals['scan_index'] = np.arange(1, self.data_set.signals.shape[0] + 1)

    def _extract_utc(self):
        """
        Extracting UTC time from dvl parsed data, merging and converting time columns to datetime utc
        """
        host_dataframe = self._parsed_data['Host']
        host_utc_time = general_extract_utc(host_dataframe, self.datetime_sigs)
        self.data_set.signals['utc_timestamp'] = host_utc_time

    def _extract_wcs_bounding_box_orientation(self):
        """
        Extracts wcs_bounding_box_orientation by converting pointing angle from degrees to radians.
        :return:
        """
        host_wcs_bounding_box_orientation_deg = self._raw_signals['AngleHeading']
        host_wcs_bounding_box_orientation_rad = np.deg2rad(host_wcs_bounding_box_orientation_deg)
        host_wcs_bounding_box_orientation_rad_normalized = normalize_angle_vector(host_wcs_bounding_box_orientation_rad)
        self.data_set.signals['wcs_bounding_box_orientation'] = host_wcs_bounding_box_orientation_rad_normalized

    def _extract_yaw_rate(self):
        """
        Extracts yaw_rate by converting pointing angle from degrees to radians.
        :return:
        """
        host_yaw_rate_deg_sec = self._raw_signals['AngRateZ']
        host_yaw_rate_rad_sec = np.deg2rad(host_yaw_rate_deg_sec)  # TODO: verify degrees/radians
        self.data_set.signals['yaw_rate'] = host_yaw_rate_rad_sec

    def _extract_timestamp(self, timestamp_convertion_factor=1e6):
        """
        Extracts timestamp signal.
        """
        self.data_set.signals['timestamp'] = self._raw_signals['timestamp'] / timestamp_convertion_factor

    def _extract_properties(self):
        """
        Extracts properties.
        :return:
        """
        self.data_set.bounding_box_dimensions_x = 4  # TODO: handle this by some config
        self.data_set.bounding_box_dimensions_y = 2

    def _extract_variances(self):
        """
        Extract variances signals
        :return:
        """
        self.data_set.signals['position_variance_x'] = RT3002Spec.pos_std ** 2
        self.data_set.signals['position_variance_y'] = RT3002Spec.pos_std ** 2
        self.data_set.signals['position_covariance'] = 0.0

        self.data_set.signals['velocity_otg_variance_x'] = RT3002Spec.vel_std ** 2
        self.data_set.signals['velocity_otg_variance_y'] = RT3002Spec.vel_std ** 2
        self.data_set.signals['velocity_otg_covariance'] = 0.0

        self.data_set.signals['yaw_rate_variance'] = RT3002Spec.angle_rate_std(self.data_set.signals['yaw_rate']) ** 2
