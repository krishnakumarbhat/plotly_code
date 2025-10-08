# coding=utf-8
"""
Rt Host Data Set interface
"""
from typing import Optional

import pandas as pd

from aspe.extractors.Interfaces.IHost import IHost
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

host_wcs_pointing_angle = SignalDescription(signature='host_wcs_pointing_angle',
                                            dtype=float,
                                            description='pointing angle (yaw/orientation angle) in world coordinate '
                                                        'system',
                                            unit='rad')


class RtHost(IHost):
    """
    Rt Host dataset interface class
    """
    def __init__(self):
        super().__init__()
        signal_names = [
            GeneralSignals.utc_timestamp,
            host_wcs_pointing_angle,
            PhysicalSignals.curvature,
        ]
        self.update_signals_definition(signal_names)
        self.bounding_box_dimensions_y: Optional[float] = None
        self.bounding_box_dimensions_x: Optional[float] = None
