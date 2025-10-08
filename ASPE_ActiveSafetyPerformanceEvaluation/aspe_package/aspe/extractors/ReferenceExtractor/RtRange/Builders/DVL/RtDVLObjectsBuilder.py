# coding=utf-8
"""
Rt-Range DVL Objects builder
"""

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtObjects import RtObjects
from aspe.extractors.Specification.reference import RT3002Spec
from aspe.utilities.DvlSupport import general_extract_utc, get_rt_extraction_possibility_flag
from aspe.utilities.MathFunctions import normalize_angle_vector
from aspe.utilities.SupportingFunctions import get_nested_array


class RtDVLObjectsBuilder(IBuilder):
    """
    Builder for Rt Objects class
    """
    # first target
    common_sigs_t1 = {
        # common data sets section
        'timestamp':                    ['Target1', 'timestamp'],
        'bounding_box_dimensions_x':    ['Target1', 'RangeTargetVehicleLength'],
        'bounding_box_dimensions_y':    ['Target1', 'RangeTargetVehicleWidth'],
        'f_iscomplete':                 ['Target1', 'f_iscomplete'],
        'position_y':                   ['Target1', 'RangePosLateral'],
        'position_x':                   ['Target1', 'RangePosForward'],
        # helping section
        # RT Range specific section
        'wcs_velocity_y':               ['Target1', 'TargetVelLateral'],
        'wcs_velocity_x':               ['Target1', 'TargetVelForward'],
        'velocity_rel_y':               ['Target1', 'RangeVelLateral'],
        'velocity_rel_x':               ['Target1', 'RangeVelForward'],
    }
    # second target
    common_sigs_t2 = {
        'timestamp':                    ['Target2', 'timestamp'],
        'bounding_box_dimensions_x':    ['Target2', 'Range2TargetVehicleLength'],
        'bounding_box_dimensions_y':    ['Target2', 'Range2TargetVehicleWidth'],
        'f_iscomplete':                 ['Target2', 'f_iscomplete'],
        'position_y':                   ['Target2', 'Range2PosLateral'],
        'position_x':                   ['Target2', 'Range2PosForward'],
        # helping section
        # RT Range specific section
        'wcs_velocity_y':               ['Target2', 'Target2VelLateral'],
        'wcs_velocity_x':               ['Target2', 'Target2VelForward'],
        'velocity_rel_y':               ['Target2', 'Range2VelLateral'],
        'velocity_rel_x':               ['Target2', 'Range2VelForward'],
    }
    # first target
    auxiliary_sigs_t1 = {
        '_yaw_rate_deg_sec':                    ['Target1', 'TargetAngRateZ'],  # TODO: verify degrees/radians
        '_wcs_bounding_box_orientation_deg':    ['Target1', 'TargetAngleHeading'],
    }
    # second target
    auxiliary_sigs_t2 = {
        '_yaw_rate_deg_sec':                    ['Target2', 'Target2AngRateZ'],  # TODO: verify degrees/radians
        '_wcs_bounding_box_orientation_deg':    ['Target2', 'Target2AngleHeading'],
    }

    datetime_sigs_t1 = {
        'century':  ['Target1', 'TargetTimeCentury'],
        'year':     ['Target1', 'TargetTimeYear'],
        'month':    ['Target1', 'TargetTimeMonth'],
        'day':      ['Target1', 'TargetTimeDay'],
        'hour':     ['Target1', 'TargetTimeHour'],
        'minute':   ['Target1', 'TargetTimeMinute'],
        'seconds':  ['Target1', 'TargetTimeSecond'],
        'ms':       ['Target1', 'TargetTimeHSecond'],
    }
    datetime_sigs_t2 = {
        'century':  ['Target2', 'Target2TimeCentury'],
        'year':     ['Target2', 'Target2TimeYear'],
        'month':    ['Target2', 'Target2TimeMonth'],
        'day':      ['Target2', 'Target2TimeDay'],
        'hour':     ['Target2', 'Target2TimeHour'],
        'minute':   ['Target2', 'Target2TimeMinute'],
        'seconds':  ['Target2', 'Target2TimeSecond'],
        'ms':       ['Target2', 'Target2TimeHSecond'],
    }

    def __init__(self, parsed_data,
                 bounding_box_dimensions_y=2,
                 bounding_box_dimensions_x=4,
                 bounding_box_refpoint_lat_offset_ratio=0.5,
                 bounding_box_refpoint_long_offset_ratio=1.0,
                 max_possible_obj_count=4,
                 f_extract_raw_signals=False,
                 f_completed_messages_only=True):
        super().__init__(parsed_data)
        self.data_set = RtObjects()

        self._bounding_box_refpoint_lat_offset_ratio = bounding_box_refpoint_lat_offset_ratio
        self._bounding_box_refpoint_long_offset_ratio = bounding_box_refpoint_long_offset_ratio
        self._bounding_box_dimensions_x = bounding_box_dimensions_x
        self._bounding_box_dimensions_y = bounding_box_dimensions_y
        self._max_possible_obj_count = max_possible_obj_count
        self._f_extract_raw_signals = f_extract_raw_signals
        self._f_completed_messages_only = f_completed_messages_only
        self.f_target1_extraction_available = get_rt_extraction_possibility_flag(self._parsed_data, 'Target1')
        self.f_target2_extraction_available = get_rt_extraction_possibility_flag(self._parsed_data, 'Target2')
        self._raw_signals = None

    def build(self):
        """
        Main build method
        :return: Rt Objects data set
        """
        self._extract_raw_signals()
        self._extract_signals()
        self._extract_variances()

        if self._f_completed_messages_only:
            self._remove_incomplete_rows()
        self._extract_properties()
        return self.data_set

    def _extract_raw_signals(self):
        if self._f_extract_raw_signals:
            self._raw_signals = pd.DataFrame()

            if self.f_target1_extraction_available:
                Target1 = self._parsed_data['Target1']
                self._raw_signals = pd.concat((self._raw_signals, Target1), axis=1)

            if self.f_target2_extraction_available:
                Target2 = self._parsed_data['Target2']
                self._raw_signals = pd.concat((self._raw_signals, Target2), axis=1)

            self.data_set.raw_signals = self._raw_signals

    def _extract_signals(self):
        """
        Extract object's default signals and add additional signals
        :return
        """
        # TODO implement in CEA-280: _extract_center, _extract_bounding_box_orientation
        if self.f_target1_extraction_available:
            # common signals
            self._extract_single_object_common_signals(unique_obj_id=0,
                                                       mapping_dict=self.common_sigs_t1)
            # mappable signals
            self._extract_single_object_scan_index(object_name='Target1', unique_obj_id=0)
            self._extract_single_object_utc(object_name='Target1',
                                            mapping_dict=self.datetime_sigs_t1,
                                            unique_obj_id=0)
            self._extract_reference_point(unique_obj_id=0)
            self._extract_bounding_box_dimensions(unique_obj_id=0)
            self._extract_single_object_wcs_bounding_box_orientation(unique_obj_id=0,
                                                                     mapping_dict=self.auxiliary_sigs_t1)

            self._extract_single_object_yaw_rate(unique_obj_id=0,
                                                 mapping_dict=self.auxiliary_sigs_t1)

        if self.f_target2_extraction_available:
            # common signals
            self._extract_single_object_common_signals(unique_obj_id=1,
                                                       mapping_dict=self.common_sigs_t2)
            # mappable signals
            self._extract_single_object_scan_index(object_name='Target2', unique_obj_id=1)
            self._extract_single_object_utc(object_name='Target2',
                                            mapping_dict=self.datetime_sigs_t2,
                                            unique_obj_id=1)
            self._extract_reference_point(unique_obj_id=1)
            self._extract_bounding_box_dimensions(unique_obj_id=1)
            self._extract_single_object_wcs_bounding_box_orientation(unique_obj_id=1,
                                                                     mapping_dict=self.auxiliary_sigs_t2)
            self._extract_single_object_yaw_rate(unique_obj_id=1,
                                                 mapping_dict=self.auxiliary_sigs_t2)
        self._postprocessing()

    def _remove_incomplete_rows(self):
        """
        Removing nan values where f_iscomplete is set to False.
        :return:
        """
        if self.data_set.signals['f_iscomplete'].empty:
            pass  # dataframe will already be clear
        else:
            self.data_set.signals = self.data_set.signals[self.data_set.signals['f_iscomplete']]

    def _extract_single_object_common_signals(self, unique_obj_id: int, mapping_dict: dict):
        """
        extracting signals from mapping signal class
        extracting only common signals without mapping and adding unique id and obj id
        """
        signals_temp = self.data_set.signals.copy()  # temp empty Dataframe only with column names

        for extracted_signal, parsed_signal in mapping_dict.items():
            signal = get_nested_array(self._parsed_data, parsed_signal)
            if signal is not None:
                signals_temp[extracted_signal] = signal

        signals_temp['id'] = unique_obj_id
        signals_temp['slot_id'] = unique_obj_id
        signals_temp['unique_id'] = unique_obj_id

        self.data_set.signals = pd.concat([self.data_set.signals, signals_temp], ignore_index=True, sort=False)
        self.data_set.signals.sort_values(by=['timestamp'])

    def _extract_single_object_utc(self, object_name: str, unique_obj_id: int, mapping_dict: dict):
        """
        Extracting UTC time from dvl parsed data, merging and converting time columns to datetime utc
        """
        signals = self.data_set.signals  # for better readability

        target_dataframe = self._parsed_data[object_name]
        target_utc_time = general_extract_utc(target_dataframe, mapping_dict)

        signals['utc_timestamp'] = signals['utc_timestamp'].where(signals['unique_id'] != unique_obj_id,
                                                                  target_utc_time).astype('datetime64[ns]')

    def _extract_single_object_wcs_bounding_box_orientation(self, unique_obj_id: int, mapping_dict: dict):
        """
        Converting pointing angle from degs to rads
        :return:
        """
        signals = self.data_set.signals  # for more convenient access

        t_wcs_bounding_box_orientation_deg = get_nested_array(self._parsed_data,
                                                              mapping_dict['_wcs_bounding_box_orientation_deg'])
        if t_wcs_bounding_box_orientation_deg is not None:
            t1_wcs_bounding_box_orientation_rad = np.deg2rad(t_wcs_bounding_box_orientation_deg)

            t1_wcs_bounding_box_orientation_normalized = normalize_angle_vector(t1_wcs_bounding_box_orientation_rad)

            t1_wcs_bounding_box_orientation_normalized = signals['wcs_bounding_box_orientation'].where(
                signals['unique_id'] != unique_obj_id, t1_wcs_bounding_box_orientation_normalized)

            signals['wcs_bounding_box_orientation'] = t1_wcs_bounding_box_orientation_normalized.astype(float)

    def _extract_single_object_yaw_rate(self, unique_obj_id: int, mapping_dict: dict):
        """
        Converting yaw rate from degs/sec to rads/sec
        :return:
        """
        signals = self.data_set.signals  # for more convenient access

        t_yaw_rate_deg_sec = get_nested_array(self._parsed_data, mapping_dict['_yaw_rate_deg_sec'])
        if t_yaw_rate_deg_sec is not None:
            t_yaw_rate_rad_sec = np.deg2rad(t_yaw_rate_deg_sec)

            signals['yaw_rate'] = signals['yaw_rate'].where(
                signals['unique_id'] != unique_obj_id, t_yaw_rate_rad_sec)

    def _extract_single_object_scan_index(self, object_name, unique_obj_id):
        """
        Generating scan indexes timeline from 1 to n and adding as scan_index column to signals
        n as lifetime length of single object
        :param object_name:
        :param unique_obj_id:
        :return:
        """
        signals = self.data_set.signals

        target_dataframe = self._parsed_data[object_name]
        scan_indexes_range = np.arange(1, target_dataframe.shape[0] + 1)
        unique_obj_mask = signals['unique_id'] == unique_obj_id

        #signals['scan_index'][unique_obj_mask] = scan_indexes_range
        signals.loc[unique_obj_mask, 'scan_index'] = scan_indexes_range

    def _extract_bounding_box_dimensions(self, unique_obj_id):
        signals = self.data_set.signals  # for more convenient access

        signals['bounding_box_dimensions_y'] = signals['bounding_box_dimensions_y'].where(
            signals['unique_id'] != unique_obj_id, self._bounding_box_dimensions_y)

        signals['bounding_box_dimensions_x'] = signals['bounding_box_dimensions_x'].where(
            signals['unique_id'] != unique_obj_id, self._bounding_box_dimensions_x)

    def _extract_reference_point(self, unique_obj_id):
        signals = self.data_set.signals  # for more convenient access

        signals['bounding_box_refpoint_lat_offset_ratio'] = signals['bounding_box_refpoint_lat_offset_ratio'].where(
            signals['unique_id'] != unique_obj_id, self._bounding_box_refpoint_lat_offset_ratio)

        signals['bounding_box_refpoint_long_offset_ratio'] = signals['bounding_box_refpoint_long_offset_ratio'].where(
            signals['unique_id'] != unique_obj_id, self._bounding_box_refpoint_long_offset_ratio)

    def _postprocessing(self, timestamp_conversion_factor=1e6):
        self.data_set.signals['timestamp'] /= timestamp_conversion_factor

    def _extract_properties(self):
        """
        Extracts properties.
        :return:
        """
        self.data_set.max_possible_obj_count = self._max_possible_obj_count
        self.data_set.coordinate_system = 'VCS'

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

        self.data_set.signals['yaw_rate_variance'] = RT3002Spec.angle_rate_std(
            self.data_set.signals['yaw_rate']) ** 2
