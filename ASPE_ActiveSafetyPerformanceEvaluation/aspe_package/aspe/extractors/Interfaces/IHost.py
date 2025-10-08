# coding=utf-8
"""
Main Host Data Set interface
"""
from typing import Optional

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.extractors.Interfaces.SignalDescription import PhysicalSignals
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

raw_speed = SignalDescription(signature='raw_speed',
                              dtype=np.float32,
                              description='Speed given to the system, negative in case of reverse movement',
                              unit='m/s')

abs_raw_speed = SignalDescription(signature='abs_raw_speed',
                                  dtype=np.float32,
                                  description='Absolute value of raw speed',
                                  unit='m/s')


class IHost(IDataSet):
    """
    Host data set interface class
    """
    def __init__(self):
        super(IHost, self).__init__()
        signal_names =[
            raw_speed,
            abs_raw_speed,
            PhysicalSignals.yaw_rate,
            PhysicalSignals.yaw_rate_variance,
            PhysicalSignals.position_x,
            PhysicalSignals.position_y,
            PhysicalSignals.velocity_otg_x,
            PhysicalSignals.velocity_otg_y,
            PhysicalSignals.velocity_otg_variance_x,
            PhysicalSignals.velocity_otg_variance_y,
            PhysicalSignals.acceleration_otg_x,
            PhysicalSignals.velocity_otg_covariance,
            PhysicalSignals.acceleration_otg_y,
        ]
        self.update_signals_definition(signal_names)
        self.coordinate_system: Optional[str] = None
        self.bounding_box_dimensions_x: Optional[float] = None
        self.bounding_box_dimensions_y: Optional[float] = None
        self.bounding_box_orientation: Optional[float] = None
        self.bounding_box_refpoint_para_offset_ratio: Optional[float] = None
        self.bounding_box_refpoint_orh_offset_ratio: Optional[float] = None
        self.dist_of_rear_axle_to_front_bumper: Optional[float] = None

    def transfer_cs_to_aptiv_vcs(self, dist_rear_axle_to_front_bumper):
        if self.coordinate_system == 'ISO_VRACS':
            self.signals.loc[:, 'center_x'] = self.signals.loc[:, 'center_x'] - dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'position_covariance'] = -self.signals.loc[:, 'position_covariance']
            self.signals.loc[:, 'center_y'] = -self.signals.loc[:, 'center_y']
            self.signals.loc[:, 'velocity_otg_y'] = -self.signals.loc[:, 'velocity_otg_y']
            self.signals.loc[:, 'velocity_otg_covariance'] = -self.signals.loc[:, 'velocity_otg_covariance']
            self.signals.loc[:, 'yaw_rate'] = -self.signals.loc[:, 'yaw_rate']
            self.signals.loc[:, 'acceleration_otg_y'] = -self.signals.loc[:, 'acceleration_otg_y']
            self.coordinate_system = 'VCS'

    def transfer_cs_to_iso_vcs(self, dist_rear_axle_to_front_bumper):
        if self.coordinate_system == 'VCS':
            self.signals.loc[:, 'center_x'] = self.signals.loc[:, 'center_x'] + dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'position_covariance'] = -self.signals.loc[:, 'position_covariance']
            self.signals.loc[:, 'center_y'] = -self.signals.loc[:, 'center_y']
            self.signals.loc[:, 'velocity_otg_y'] = -self.signals.loc[:, 'velocity_otg_y']
            self.signals.loc[:, 'velocity_otg_covariance'] = -self.signals.loc[:, 'velocity_otg_covariance']
            self.signals.loc[:, 'yaw_rate'] = -self.signals.loc[:, 'yaw_rate']
            self.signals.loc[:, 'acceleration_otg_y'] = -self.signals.loc[:, 'acceleration_otg_y']
            self.signals.loc[:, 'bounding_box_orientation'] = -self.signals.loc[:, 'bounding_box_orientation']
            self.coordinate_system = 'ISO_VRACS'

    def get_base_name(self):
        """
        Get base name of Host interface
        :return: str: Host base data set name
        """
        return 'host'
