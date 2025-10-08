# coding=utf-8
"""
Rt Objects dataset interface
"""
from datetime import datetime
from typing import Optional

import pandas as pd

from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

f_iscomplete = SignalDescription(signature='f_iscomplete',
                                 dtype=bool,
                                 description='Information if all required signals are available',
                                 unit='-')

wcs_bounding_box_orientation = SignalDescription(signature='wcs_bounding_box_orientation',
                                                 dtype=float,
                                                 description='orientation angle (yaw/pointing angle) in world'
                                                             ' coordinate system',
                                                 unit='rad')

acceleration_tcs_x = SignalDescription(signature='acceleration_tcs_x',
                                       dtype=float,
                                       description='Over-The-Ground acceleration in x coordinate in'
                                                   'target coordinate system',
                                       unit='m^2/s^2')

acceleration_tcs_y = SignalDescription(signature='acceleration_tcs_y',
                                       dtype=float,
                                       description='Over-The-Ground acceleration in y coordinate in'
                                                   'target coordinate system',
                                       unit='m^2/s^2')

wcs_velocity_y = SignalDescription(signature='wcs_velocity_y',
                                   dtype=float,
                                   description='Over-The-Ground velocity in x coordinate in world coordinate system',
                                   unit='m/s')

wcs_velocity_x = SignalDescription(signature='wcs_velocity_x',
                                   dtype=float,
                                   description='Over-The-Ground velocity in y coordinate in world coordinate system',
                                   unit='m/s')


class RtObjects(IObjects):
    """
    Rt Objects dataset class
    """
    def __init__(self):
        super().__init__()
        signal_names = [
            f_iscomplete,
            GeneralSignals.utc_timestamp,
            wcs_bounding_box_orientation,
            acceleration_tcs_x,
            acceleration_tcs_y,
            wcs_velocity_y,
            wcs_velocity_x,
            PhysicalSignals.curvature,
            PhysicalSignals.yaw_acceleration,
        ]
        self.update_signals_definition(signal_names)
        self.max_possible_obj_count: Optional[int] = None
        self.coordinate_system: Optional[str] = None
