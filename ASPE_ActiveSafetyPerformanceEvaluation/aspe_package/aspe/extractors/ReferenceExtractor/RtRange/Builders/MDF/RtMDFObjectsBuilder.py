from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtObjects import RtObjects
from aspe.utilities.MathFunctions import normalize_angle_vector
from aspe.utilities.mdf_support import align_rt_messages, get_full_master_name, mdf_rt_extract_utc
from aspe.utilities.SupportingFunctions import recursive_dict_extraction


class RtMDFObjectsBuilder(IBuilder):
    """
    Builder for Rt Objects class
    """
    master_message = {'Target1': 'RangeForward_RangeForward_timestamp',
                      'Target2': 'Range2Forward_Range2Forward_timestamp', }
    signal_mapper = {
        'Target1': {
            # ASPE signature              RT-Range signature
            'position_y':                'RangeLateral_RangePosLateral',
            'position_x':                'RangeForward_RangePosForward',
            'timestamp':                 'RangeForward_RangeForward_timestamp',
            'bounding_box_dimensions_x': 'RangeVehicle_RangeTargetVehicleLength',
            'bounding_box_dimensions_y': 'RangeVehicle_RangeTargetVehicleWidth',
            'velocity_rel_y':            'RangeLateral_RangeVelLateral',
            'velocity_rel_x':            'RangeForward_RangeVelForward',
            'wcs_velocity_y':            'TargetVelocityLevel_TargetVelLateral',
            'wcs_velocity_x':            'TargetVelocityLevel_TargetVelForward',
            'acceleration_tcs_x':        'TargetAccelLevel_TargetAccelForward',
            'acceleration_tcs_y':        'TargetAccelLevel_TargetAccelLateral',
            'curvature':                 'TargetTrackSlipCurvature_TargetCurvature',
    },
        'Target2': {
            'position_y':                'Range2Lateral_Range2PosLateral',
            'position_x':                'Range2Forward_Range2PosForward',
            'timestamp':                 'Range2Forward_Range2Forward_timestamp',
            'bounding_box_dimensions_x': 'Range2Vehicle_Range2TargetVehicleLength',
            'bounding_box_dimensions_y': 'Range2Vehicle_Range2TargetVehicleWidth',
            'velocity_rel_y':            'Range2Lateral_Range2VelLateral',
            'velocity_rel_x':            'Range2Forward_Range2VelForward',
            'wcs_velocity_y':            'Target2VelocityLevel_Target2VelLateral',
            'wcs_velocity_x':            'Target2VelocityLevel_Target2VelForward',
        },
    }
    # first target
    auxiliary_sigs_t1 = {
        '_yaw_rate_deg_sec':                 'TargetRateVehicle_TargetAngRateZ',  # TODO: verify degrees/radians
        '_wcs_bounding_box_orientation_deg': 'TargetHeadingPitchRoll_TargetAngleHeading',
    }
    # second target
    auxiliary_sigs_t2 = {
        '_yaw_rate_deg_sec':                 'Target2RateVehicle_Target2AngRateZ',  # TODO: verify degrees/radians
        '_wcs_bounding_box_orientation_deg': 'Target2HeadingPitchRoll_Target2AngleHeading',
    }

    mapping_utc_signals = {
        'Target1':
            [
                'TargetDateTime_TargetTimeCentury',
                'TargetDateTime_TargetTimeYear',
                'TargetDateTime_TargetTimeMonth',
                'TargetDateTime_TargetTimeDay',
                'TargetDateTime_TargetTimeHour',
                'TargetDateTime_TargetTimeMinute',
                'TargetDateTime_TargetTimeSecond',
                'TargetDateTime_TargetTimeHSecond',
            ],

        'Target2':
            [
                'Target2DateTime_Target2TimeCentury',
                'Target2DateTime_Target2TimeYear',
                'Target2DateTime_Target2TimeMonth',
                'Target2DateTime_Target2TimeDay',
                'Target2DateTime_Target2TimeHour',
                'Target2DateTime_Target2TimeMinute',
                'Target2DateTime_Target2TimeSecond',
                'Target2DateTime_Target2TimeHSecond',
            ],
    }

    def __init__(self, parsed_data, master_sub_name,
                 bounding_box_refpoint_lat_offset_ratio=0.5,
                 bounding_box_refpoint_long_offset_ratio=1.0,
                 max_possible_obj_count=4,
                 f_extract_raw_signals=True,
                 f_completed_messages_only=True):
        super().__init__(parsed_data)
        self.master_sub_name = master_sub_name
        self.data_set = RtObjects()
        self._bounding_box_refpoint_lat_offset_ratio = bounding_box_refpoint_lat_offset_ratio
        self._bounding_box_refpoint_long_offset_ratio = bounding_box_refpoint_long_offset_ratio
        self._max_possible_obj_count = max_possible_obj_count
        self._f_extract_raw_signals = f_extract_raw_signals
        self._f_completed_messages_only = f_completed_messages_only
        self.f_target1_extraction_available = 'Target1' in self._parsed_data.keys()
        self.f_target2_extraction_available = 'Target2' in self._parsed_data.keys()
        self._raw_signals = None

    def build(self):
        """
        Main build method
        :return: Rt Objects data set
        """
        self._extract_raw_signals()
        self._extract_signals()
        if self._f_completed_messages_only:
            self._remove_incomplete_rows()
        self._extract_properties()
        self._remove_empty_dimensions_cells()
        return self.data_set

    def _extract_raw_signals(self):
        self._raw_signals = pd.DataFrame()
        if self.f_target1_extraction_available:
            target1 = recursive_dict_extraction(self._parsed_data['Target1'])[0]
            self.master_message['Target1'] = get_full_master_name(target1, self.master_sub_name)
            aligned_target1 = align_rt_messages(target1, self.master_message['Target1'])
            self._raw_signals = pd.concat((self._raw_signals, pd.DataFrame(aligned_target1)), axis=1)

        if self.f_target2_extraction_available:
            target2 = recursive_dict_extraction(self._parsed_data['Target2'])[0]
            self.master_message['Target2'] = get_full_master_name(target2, self.master_sub_name)
            aligned_target2 = align_rt_messages(target2, self.master_message['Target2'])
            self._raw_signals = pd.concat((self._raw_signals, aligned_target2), axis=1)
        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_signals(self):
        """
        Extract object's default signals and add additional signals
        :return
        """
        # TODO implement in CEA-280: _extract_center, _extract_bounding_box_orientation
        if self.f_target1_extraction_available:
            self._extract_mappable_signals('Target1', unique_id=0)
            self._extract_single_object_scan_index(object_name='Target1', unique_obj_id=0)
            self._extract_single_object_utc(object_name='Target1', unique_obj_id=0)
            self._extract_reference_point(unique_obj_id=0)
            self._extract_single_object_wcs_bounding_box_orientation(unique_obj_id=0,
                                                                     mapping_dict=self.auxiliary_sigs_t1)
            self._extract_single_object_yaw_rate(unique_obj_id=0,
                                                 mapping_dict=self.auxiliary_sigs_t1)

        if self.f_target2_extraction_available:
            # common signals
            self._extract_mappable_signals('Target2', unique_id=1)
            self._extract_single_object_scan_index(object_name='Target2', unique_obj_id=1)
            self._extract_single_object_utc(object_name='Target2', unique_obj_id=1)
            self._extract_reference_point(unique_obj_id=1)
            self._extract_single_object_wcs_bounding_box_orientation(unique_obj_id=1,
                                                                     mapping_dict=self.auxiliary_sigs_t2)
            self._extract_single_object_yaw_rate(unique_obj_id=1,
                                                 mapping_dict=self.auxiliary_sigs_t2)
        self._extract_yaw_acceleration()
        self.data_set.coordinate_system = 'VCS'

    def _extract_mappable_signals(self, target_name: str, unique_id: int):
        """
        Extract mappable signals from raw signals.
        :return:
        """
        signals = pd.DataFrame(columns=self.data_set.signals.columns)
        for aspe_signature, raw_signature in self.signal_mapper[target_name].items():
            try:
                signals[aspe_signature] = self._raw_signals[raw_signature]
            except KeyError:
                warn(f'Signal {raw_signature} not found, {aspe_signature} will not been extracted')

        signals['id'] = unique_id
        signals['slot_id'] = unique_id
        signals['unique_id'] = unique_id

        self.data_set.signals = pd.concat([self.data_set.signals, signals],
                                          ignore_index=True,
                                          sort=False).reset_index(drop=True)
        self.data_set.signals.sort_values(by=['timestamp'])


    def _remove_incomplete_rows(self):
        """
        Remove rows with nan values excluding columns filled with nan
        :return:
        """
        empty_columns = []
        for column_name, column_data in self.data_set.signals.iteritems():
            if column_data.isnull().all():
                empty_columns.append(column_name)

        reduced = self.data_set.signals.drop(columns=empty_columns)
        indexes = reduced.dropna().index
        self.data_set.signals = self.data_set.signals.loc[indexes].reset_index(drop=True)
        self._raw_signals = self._raw_signals.loc[indexes].reset_index(drop=True)

    def _extract_single_object_utc(self, object_name: str, unique_obj_id: int):
        """
        Extracting UTC time from dvl parsed data, merging and converting time columns to datetime utc
        """
        signals = self.data_set.signals  # for better readability

        target_dataframe = self._raw_signals
        target_utc_time = mdf_rt_extract_utc(target_dataframe, self.mapping_utc_signals[object_name])

        signals['utc_timestamp'] = signals['utc_timestamp'].where(signals['unique_id'] != unique_obj_id,
                                                                  target_utc_time).astype('datetime64[ns]')

    def _extract_single_object_wcs_bounding_box_orientation(self, unique_obj_id: int, mapping_dict: dict):
        """
        Converting pointing angle from degs to rads
        :return:
        """
        signals = self.data_set.signals  # for more convenient access

        t_wcs_bounding_box_orientation_deg = self._raw_signals[mapping_dict['_wcs_bounding_box_orientation_deg']]
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

        t_yaw_rate_deg_sec = self._raw_signals[mapping_dict['_yaw_rate_deg_sec']]
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
        target_dataframe = self._raw_signals[self.master_message[object_name]]
        scan_indexes_range = np.arange(1, len(target_dataframe) + 1)
        unique_obj_mask = signals['unique_id'] == unique_obj_id
        signals.loc[unique_obj_mask, 'scan_index'] = scan_indexes_range

    def _extract_reference_point(self, unique_obj_id):
        signals = self.data_set.signals  # for more convenient access

        signals['bounding_box_refpoint_lat_offset_ratio'] = signals['bounding_box_refpoint_lat_offset_ratio'].where(
            signals['unique_id'] != unique_obj_id, self._bounding_box_refpoint_lat_offset_ratio)

        signals['bounding_box_refpoint_long_offset_ratio'] = signals['bounding_box_refpoint_long_offset_ratio'].where(
            signals['unique_id'] != unique_obj_id, self._bounding_box_refpoint_long_offset_ratio)

    def _extract_properties(self):
        """
        Extracts properties.
        :return:
        """
        self.data_set.max_possible_obj_count = self._max_possible_obj_count

    def _remove_empty_dimensions_cells(self):
        length = self.data_set.signals['bounding_box_dimensions_x'].to_numpy()
        width = self.data_set.signals['bounding_box_dimensions_y'].to_numpy()
        length = length[~np.isnan(length)]
        width = width[~np.isnan(width)]
        if len(length) > 0:
            self.data_set.signals['bounding_box_dimensions_x'] = length[0]
        if len(width) > 0:
            self.data_set.signals['bounding_box_dimensions_y'] = width[0]

    def _extract_yaw_acceleration(self):
        lateral_acc = self.data_set.signals.loc[:, 'acceleration_tcs_y']
        curvature = self.data_set.signals.loc[:, 'curvature'].to_numpy().astype(float)
        if np.any(np.isnan(curvature)):  # in lot of cases curvature signals contains lot of drops
            curvature = np.where(np.isnan(curvature), 0.0, curvature)
            self.data_set.signals.loc[:, 'curvature'] = curvature
        yaw_acceleration = lateral_acc * curvature
        self.data_set.signals.loc[:, 'yaw_acceleration'] = yaw_acceleration
