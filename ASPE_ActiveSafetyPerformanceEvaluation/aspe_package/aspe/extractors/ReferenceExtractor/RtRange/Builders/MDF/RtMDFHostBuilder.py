"""
Rt-Range MDF Host builder
"""
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtHost import RtHost
from aspe.extractors.Specification.reference import RT3002Spec
from aspe.utilities.MathFunctions import calc_acceleration_in_position, normalize_angle_vector
from aspe.utilities.mdf_support import align_rt_messages, get_full_master_name, mdf_rt_extract_utc
from aspe.utilities.SupportingFunctions import recursive_dict_extraction


class RtMDFHostBuilder(IBuilder):
    """
    Rt-Range MDF Host builder class
    """

    signal_mapper = {
        # ASPE signature      RT-Range signature
        'position_y':         'LatitudeLongitude_PosLat',
        'position_x':         'LatitudeLongitude_PosLon',
        'velocity_otg_y':     'VelocityLevel_VelLateral',
        'velocity_otg_x':     'VelocityLevel_VelForward',
        'acceleration_otg_x': 'AccelLevel_AccelForward',
        'acceleration_otg_y': 'AccelLevel_AccelLateral',
        'curvature':          'TrackSlipCurvature_Curvature',
        'raw_speed':          'Velocity_Speed2D',
        'timestamp':          'DateTime_DateTime_timestamp',
    }

    mapping_utc_signals = [
                'DateTime_TimeCentury',
                'DateTime_TimeYear',
                'DateTime_TimeMonth',
                'DateTime_TimeDay',
                'DateTime_TimeHour',
                'DateTime_TimeMinute',
                'DateTime_TimeSecond',
                'DateTime_TimeHSecond',
    ]

    def __init__(self,
                 parsed_data,
                 master_sub_name,
                 host_length=4,
                 host_width=2,
                 f_extract_raw_signals=False,
                 f_completed_messages_only=True,
                 rear_axle_to_front_bumper_dist=None):

        super().__init__(parsed_data)
        self.data_set = RtHost()
        self._host_length = host_length
        self._host_width = host_width
        self._f_extract_raw_signals = f_extract_raw_signals
        self._f_completed_messages_only = f_completed_messages_only
        self._rear_axle_to_front_bumper_dist = rear_axle_to_front_bumper_dist
        self._raw_signals = pd.DataFrame()
        self.master_sub_name = master_sub_name

    def build(self):
        """
        Main build method.
        :return: RT Host data set
        """
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()

        if self._f_completed_messages_only:
            self._remove_incomplete_rows()

        return self.data_set

    def _extract_raw_signals(self):
        """
        Extract raw signals from parsed data.
        :return:
        """
        flattened_host = recursive_dict_extraction(self._parsed_data['Host'])[0]
        self.master_message = get_full_master_name(flattened_host, self.master_sub_name)
        aligned_host = align_rt_messages(flattened_host, self.master_message)
        for name, values in aligned_host.items():
            if "Gps" not in name:
                self._raw_signals[name] = values

        self._raw_signals.sort_index(axis=1, inplace=True)

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_mappable_signals(self):
        """
        Extract mappable signals from raw signals.
        :return:
        """
        for aspe_signature, raw_signature in self.signal_mapper.items():
            try:
                self.data_set.signals[aspe_signature] = self._raw_signals[raw_signature]
            except KeyError:
                warn(f'Signal {raw_signature} not found, {aspe_signature} will not been extracted')

    def _extract_non_mappable_signals(self):
        """
        Extract signals that cannot be directly mapped.
        :return:
        """
        self.data_set.signals['id'] = 0
        self.data_set.signals['slot_id'] = 0
        self.data_set.signals['unique_id'] = 0

        self._extract_single_object_scan_index()
        self._extract_utc()
        self._extract_wcs_bounding_box_orientation()
        self._extract_yaw_rate()
        self._extract_variances()
        self._extract_yaw_acceleration()
        self._translate_acceleration_to_reference_point()

    def _extract_single_object_scan_index(self):
        """
        Generate scan indexes timeline from 1 to n and adding as scan_index column to signals
        :return:
        """
        self.data_set.signals['scan_index'] = np.arange(1, self.data_set.signals.shape[0] + 1)

    def _extract_utc(self):
        """
        Extract UTC time from dvl parsed data, merge and convert time columns to datetime utc
        """
        host_dataframe = self._raw_signals
        host_utc_time = mdf_rt_extract_utc(host_dataframe, self.mapping_utc_signals)
        self.data_set.signals['utc_timestamp'] = host_utc_time

    def _extract_wcs_bounding_box_orientation(self):
        """
        Extract wcs_bounding_box_orientation by converting pointing angle from degrees to radians.
        :return:
        """
        host_wcs_bounding_box_orientation_deg = self._raw_signals['HeadingPitchRoll_AngleHeading']
        host_wcs_bounding_box_orientation_rad = np.deg2rad(host_wcs_bounding_box_orientation_deg)
        host_wcs_bounding_box_orientation_rad_normalized = normalize_angle_vector(host_wcs_bounding_box_orientation_rad)
        self.data_set.signals['wcs_bounding_box_orientation'] = host_wcs_bounding_box_orientation_rad_normalized

    def _extract_yaw_rate(self):
        """
        Extracts yaw_rate by converting pointing angle from degrees to radians.
        :return:
        """
        host_yaw_rate_deg_sec = self._raw_signals['RateVehicle_AngRateZ']
        host_yaw_rate_rad_sec = np.deg2rad(host_yaw_rate_deg_sec)
        host_yaw_rate_rad_sec_normalized = normalize_angle_vector(host_yaw_rate_rad_sec)
        self.data_set.signals['yaw_rate'] = host_yaw_rate_rad_sec_normalized

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

    def _extract_properties(self):
        """
        Extract properties.
        :return:
        """
        self.data_set.bounding_box_dimensions_x = self._host_length
        self.data_set.bounding_box_dimensions_y = self._host_width
        if self._rear_axle_to_front_bumper_dist is not None:
            self.data_set.dist_of_rear_axle_to_front_bumper = self._rear_axle_to_front_bumper_dist

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
        self.data_set.signals = self.data_set.signals.loc[indexes]
        self._raw_signals = self._raw_signals.loc[indexes]

    def _extract_yaw_acceleration(self):
        lateral_acc = self.data_set.signals.loc[:, 'acceleration_otg_y']
        curvature = self.data_set.signals.loc[:, 'curvature'].to_numpy().astype(float)
        if np.any(np.isnan(curvature)):  # in lot of cases curvature signals contains lot of drops
            curvature = np.where(np.isnan(curvature), 0.0, curvature)
            self.data_set.signals.loc[:, 'curvature'] = curvature
        yaw_acceleration = lateral_acc * curvature
        self.data_set.signals.loc[:, 'yaw_acceleration'] = yaw_acceleration

    def _translate_acceleration_to_reference_point(self):
        signals = self.data_set.signals
        acc_x = signals.loc[:, 'acceleration_otg_x'].copy().to_numpy()
        acc_y = signals.loc[:, 'acceleration_otg_y'].copy().to_numpy()
        yaw_rate = signals.loc[:, 'yaw_rate']
        yaw_acc = signals.loc[:, 'yaw_acceleration']
        rt_box_pos_ratio_x, rt_box_pos_ratio_y = 0.3, 0.5  # TODO this should be set in class init as arg
        ref_point_pos_ratio_x, ref_point_pos_ratio_y = 1.0, 0.5  # TODO this should be set in class init as arg
        rt_box_pos_x, rt_box_pos_y = rt_box_pos_ratio_x * self._host_length, rt_box_pos_ratio_y * self._host_width
        new_pos_x, new_pos_y = ref_point_pos_ratio_x * self._host_length, ref_point_pos_ratio_y * self._host_width
        new_acc_x, new_acc_y = calc_acceleration_in_position(rt_box_pos_x, rt_box_pos_y, acc_x, acc_y,
                                                             yaw_rate, yaw_acc, new_pos_x, new_pos_y)
        signals.loc[:, 'acceleration_otg_x'] = new_acc_x
        signals.loc[:, 'acceleration_otg_y'] = new_acc_y
