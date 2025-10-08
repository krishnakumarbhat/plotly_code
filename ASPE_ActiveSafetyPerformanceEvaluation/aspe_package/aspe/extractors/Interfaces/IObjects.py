# coding=utf-8
"""
Main Radar Tracker Object's data set interface
"""

from typing import Optional

import pandas as pd

from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.extractors.Transform.cs_transform_cov import rotate_2d_cov
from aspe.utilities.MathFunctions import normalize_angle_vector, rot_2d_sae_cs, vcs2scs


class IObjects(IDataSet):
    """
    Radar Tracker Object's data set interface class
    """

    def __init__(self):
        super(IObjects, self).__init__()
        # TODO all signals shall be described
        signal_names = [
            GeneralSignals.id,
            PhysicalSignals.position_x,
            PhysicalSignals.position_y,
            PhysicalSignals.position_variance_x,
            PhysicalSignals.position_variance_y,
            PhysicalSignals.position_covariance,
            PhysicalSignals.velocity_otg_x,
            PhysicalSignals.velocity_otg_y,
            PhysicalSignals.velocity_otg_variance_x,
            PhysicalSignals.velocity_otg_variance_y,
            PhysicalSignals.velocity_otg_covariance,
            PhysicalSignals.velocity_rel_x,  # TODO: remove, should be custom, not common
            PhysicalSignals.velocity_rel_y,  # TODO: remove, should be custom, not common
            PhysicalSignals.acceleration_otg_x,
            PhysicalSignals.acceleration_otg_y,
            PhysicalSignals.acceleration_rel_x,  # TODO: remove, should be custom, not common
            PhysicalSignals.acceleration_rel_y,  # TODO: remove, should be custom, not common
            PhysicalSignals.acceleration_otg_variance_x,
            PhysicalSignals.acceleration_otg_variance_y,
            PhysicalSignals.acceleration_otg_covariance,
            PhysicalSignals.bounding_box_dimensions_x,
            PhysicalSignals.bounding_box_dimensions_y,
            PhysicalSignals.bounding_box_orientation,
            PhysicalSignals.bounding_box_refpoint_lat_offset_ratio,
            PhysicalSignals.bounding_box_refpoint_long_offset_ratio,
            GeneralSignals.movement_status,
            GeneralSignals.motion_model_type,
            GeneralSignals.motion_model_state_suppl,
            GeneralSignals.motion_model_variances_suppl,
            GeneralSignals.motion_model_covariances_suppl,
            GeneralSignals.object_class,
            GeneralSignals.object_class_probability,
            GeneralSignals.existence_indicator,
            PhysicalSignals.speed,
            PhysicalSignals.yaw_rate,
            PhysicalSignals.center_x,
            PhysicalSignals.center_y,
        ]
        self.update_signals_definition(signal_names)
        self.max_possible_obj_count: Optional[int] = None
        self.coordinate_system: Optional[str] = None

    def _transfer_to_vcs(self, dist_rear_axle_to_front_bumper):
        if self.coordinate_system == 'ISO_VRACS':
            self.signals.loc[:, 'position_x'] = self.signals.loc[:, 'position_x'] - dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'center_x'] = self.signals.loc[:, 'center_x'] - dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'position_y'] = -self.signals.loc[:, 'position_y']
            self.signals.loc[:, 'position_covariance'] = -self.signals.loc[:, 'position_covariance']
            self.signals.loc[:, 'center_y'] = -self.signals.loc[:, 'center_y']
            self.signals.loc[:, 'velocity_otg_y'] = -self.signals.loc[:, 'velocity_otg_y']
            self.signals.loc[:, 'velocity_otg_covariance'] = -self.signals.loc[:, 'velocity_otg_covariance']
            self.signals.loc[:, 'velocity_rel_y'] = -self.signals.loc[:, 'velocity_rel_y']
            self.signals.loc[:, 'yaw_rate'] = -self.signals.loc[:, 'yaw_rate']
            self.signals.loc[:, 'acceleration_otg_y'] = -self.signals.loc[:, 'acceleration_otg_y']
            self.signals.loc[:, 'acceleration_rel_y'] = -self.signals.loc[:, 'acceleration_rel_y']
            self.signals.loc[:, 'acceleration_otg_covariance'] = -self.signals.loc[:, 'acceleration_otg_covariance']
            self.signals.loc[:, 'bounding_box_orientation'] = -self.signals.loc[:, 'bounding_box_orientation']
            self.signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio'] = \
                1.0 - self.signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio']
            self.coordinate_system = 'VCS'

        if self.coordinate_system == 'PANCS':
            self.signals.loc[:, 'position_x'] = self.signals.loc[:, 'position_x'] - dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'center_x'] = self.signals.loc[:, 'center_x'] - dist_rear_axle_to_front_bumper
            self.coordinate_system = 'VCS'

    def _transfer_to_iso_vracs(self, dist_rear_axle_to_front_bumper):
        if self.coordinate_system == 'VCS':
            self.signals.loc[:, 'position_x'] = self.signals.loc[:, 'position_x'] + dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'center_x'] = self.signals.loc[:, 'center_x'] + dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'position_y'] = -self.signals.loc[:, 'position_y']
            self.signals.loc[:, 'position_covariance'] = -self.signals.loc[:, 'position_covariance']
            self.signals.loc[:, 'center_y'] = -self.signals.loc[:, 'center_y']
            self.signals.loc[:, 'velocity_otg_y'] = -self.signals.loc[:, 'velocity_otg_y']
            self.signals.loc[:, 'velocity_otg_covariance'] = -self.signals.loc[:, 'velocity_otg_covariance']
            self.signals.loc[:, 'velocity_rel_y'] = -self.signals.loc[:, 'velocity_rel_y']
            self.signals.loc[:, 'yaw_rate'] = -self.signals.loc[:, 'yaw_rate']
            self.signals.loc[:, 'acceleration_otg_y'] = -self.signals.loc[:, 'acceleration_otg_y']
            self.signals.loc[:, 'acceleration_rel_y'] = -self.signals.loc[:, 'acceleration_rel_y']
            self.signals.loc[:, 'acceleration_otg_covariance'] = -self.signals.loc[:, 'acceleration_otg_covariance']
            self.signals.loc[:, 'bounding_box_orientation'] = -self.signals.loc[:, 'bounding_box_orientation']
            self.signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio'] = \
                1.0 - self.signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio']
            self.coordinate_system = 'ISO_VRACS'

    def get_base_name(self):
        """
        Get objects base name
        :return: objects base name
        """
        return 'objects'

    def vcs2scs(self, scs_location):
        if not self.coordinate_system == 'VCS':
            raise ValueError(f"Object can be converted only if it is in VCS, current coordinate is: "
                             f"{self.coordinate_system}")
        object_signals = self.signals

        columns_to_change = ['position_x',
                             'position_y',
                             'position_variance_x',
                             'position_variance_y',
                             'position_covariance',
                             'velocity_otg_x',
                             'velocity_otg_y',
                             'velocity_otg_variance_x',
                             'velocity_otg_variance_y',
                             'velocity_otg_covariance',
                             'acceleration_otg_x',
                             'acceleration_otg_y',
                             'acceleration_otg_variance_x',
                             'acceleration_otg_variance_y',
                             'acceleration_otg_covariance',
                             'bounding_box_orientation',
                             'bounding_box_pseudo_variance_orientation_local',
                             'speed',
                             'center_x',
                             'center_y',
                             ]
        columns_to_keep = object_signals.columns.difference(columns_to_change)
        signals_scs = pd.DataFrame(columns=object_signals.columns)
        signals_scs[columns_to_keep] = object_signals[columns_to_keep]

        scs_x, scs_y = vcs2scs(object_signals['position_x'], object_signals['position_y'],
                               scs_location.position_x, scs_location.position_y, scs_location.boresight_az_angle)

        scs_center_x, scs_center_y = vcs2scs(object_signals['center_x'], object_signals['center_y'],
                                             scs_location.position_x, scs_location.position_y,
                                             scs_location.boresight_az_angle)

        scs_velocity_otg_x, scs_velocity_otg_y = rot_2d_sae_cs(object_signals['velocity_otg_x'],
                                                               object_signals['velocity_otg_y'],
                                                               scs_location.boresight_az_angle)

        scs_x_var, scs_y_var, scs_xy_cov = rotate_2d_cov(object_signals['position_variance_x'],
                                                         object_signals['position_variance_y'],
                                                         object_signals['position_covariance'],
                                                         -scs_location.boresight_az_angle)

        scs_vx_var, scs_vy_var, scs_vxvy_cov = rotate_2d_cov(object_signals['velocity_otg_variance_x'],
                                                             object_signals['velocity_otg_variance_y'],
                                                             object_signals['velocity_otg_covariance'],
                                                             -scs_location.boresight_az_angle)

        signals_scs['bounding_box_orientation'] = \
            normalize_angle_vector(object_signals['bounding_box_orientation'] - scs_location.boresight_az_angle)

        signals_scs['position_x'] = scs_x
        signals_scs['position_y'] = scs_y

        signals_scs['position_variance_x'] = scs_x_var
        signals_scs['position_variance_y'] = scs_y_var
        signals_scs['position_covariance'] = scs_xy_cov

        signals_scs['velocity_otg_x'] = scs_velocity_otg_x
        signals_scs['velocity_otg_y'] = scs_velocity_otg_y

        signals_scs['velocity_otg_variance_x'] = scs_vx_var
        signals_scs['velocity_otg_variance_y'] = scs_vy_var
        signals_scs['velocity_otg_covariance'] = scs_vxvy_cov

        signals_scs['center_x'] = scs_center_x
        signals_scs['center_y'] = scs_center_y

        self.signals = signals_scs
        self.coordinate_system = 'SCS'
