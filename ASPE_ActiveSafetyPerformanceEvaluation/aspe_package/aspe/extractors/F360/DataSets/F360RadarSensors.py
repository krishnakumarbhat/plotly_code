# coding=utf-8
"""
F360 Radar Sensors Data Set
"""

import pandas as pd

from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors


class F360RadarSensors(IRadarSensors):
    """
    F360 Radar Sensors data set class
    """

    def __init__(self, f_reduced_sensor_stream=False):
        super().__init__()
        # F360 Sensor stream reduced to log once every 100th index.
        # No per_look data available, instead update per_sensor data
        if f_reduced_sensor_stream:
            del self.per_look
            self.per_sensor = pd.DataFrame(columns={'sensor_id',
                                                    'sensor_type',
                                                    'polarity',
                                                    'mount_location',
                                                    'boresight_az_angle',
                                                    'boresight_elev_angle',
                                                    'position_x',
                                                    'position_y',
                                                    'position_z',
                                                    'max_range',
                                                    'min_elevation',
                                                    'max_elevation',
                                                    'min_azimuth',
                                                    'max_azimuth',
                                                    'v_wrapping',
                                                    'r_wrapping'})
