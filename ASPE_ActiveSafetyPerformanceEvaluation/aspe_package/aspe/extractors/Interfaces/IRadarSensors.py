# coding=utf-8
"""
Main Radar Sensors interface
"""
from typing import Optional

import pandas as pd

from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.extractors.Transform.cs_transform_cov import rotate_2d_cov
from aspe.utilities.data_filters import slice_df_based_on_other
from aspe.utilities.MathFunctions import rot_2d_sae_cs


class IRadarSensors(IDataSet):
    """
    Main Radar Sensors data set interface class
    """
    def __init__(self):
        super(IRadarSensors, self).__init__()
        signal_names = [
            GeneralSignals.sensor_id,
            GeneralSignals.look_id,
            GeneralSignals.look_index,
            GeneralSignals.sensor_valid,
            GeneralSignals.new_measurement_update,
            PhysicalSignals.velocity_otg_x,
            PhysicalSignals.velocity_otg_y,
            PhysicalSignals.yaw_rate,
        ]
        self.update_signals_definition(signal_names)

        self.coordinate_system: Optional[str] = None
        self.per_sensor = pd.DataFrame(columns=['sensor_id',
                                                'sensor_type',
                                                'polarity',
                                                'mount_location',
                                                'boresight_az_angle',
                                                'boresight_elev_angle',
                                                'position_x',
                                                'position_y',
                                                'position_z'])
        # All look properties are in SCS
        self.per_look = pd.DataFrame(columns=['sensor_id',
                                              'look_id',
                                              'max_range',
                                              'min_elevation',
                                              'max_elevation',
                                              'min_azimuth',
                                              'max_azimuth',
                                              'v_wrapping',
                                              'r_wrapping'])

    def _transfer_to_iso_vracs(self, dist_rear_axle_to_front_bumper):
        if self.coordinate_system == 'VCS':
            self.per_sensor.loc[:, 'position_x'] = self.per_sensor.loc[:, 'position_x'] + dist_rear_axle_to_front_bumper
            self.per_sensor.loc[:, 'position_y'] = -self.per_sensor.loc[:, 'position_y']
            self.per_sensor.loc[:, 'boresight_az_angle'] = -self.per_sensor.loc[:, 'boresight_az_angle']
            self.coordinate_system = 'ISO_VRACS'

    def _transfer_to_vcs(self, dist_rear_axle_to_front_bumper):
        if self.coordinate_system == 'ISO_VRACS':
            self.per_sensor.loc[:, 'position_x'] = self.per_sensor.loc[:, 'position_x'] - dist_rear_axle_to_front_bumper
            self.per_sensor.loc[:, 'position_y'] = -self.per_sensor.loc[:, 'position_y']
            self.per_sensor.loc[:, 'boresight_az_angle'] = -self.per_sensor.loc[:, 'boresight_az_angle']
            self.coordinate_system = 'VCS'

    def get_base_name(self):
        """
        Get sensors base name
        :return: sensors base name
        """
        return 'sensors'

    def vcs2scs(self):
        """
        Conversion from VCS coordinate to SCS coordinate.
        It takes mounting location from .per_sensor DataFrame
        It affects:
            self.signals['velocity_otg_x']
            self.signals['velocity_otg_y']
            self.signals['velocity_otg_variance_x']
            self.signals['velocity_otg_variance_y']
            self.signals['velocity_otg_covariance']
        :return:
        """
        if not self.coordinate_system == 'VCS':
            raise ValueError(f"Object can be converted only if it is in VCS, current coordinate is: "
                             f"{self.coordinate_system}")

        sensor_mounting_synch = slice_df_based_on_other(self.per_sensor, self.signals, ['sensor_id'], ['sensor_id'])
        boresight_az_angle = sensor_mounting_synch['boresight_az_angle']

        self.signals['velocity_otg_x'], self.signals['velocity_otg_y'] = rot_2d_sae_cs(self.signals['velocity_otg_x'],
                                                                                       self.signals['velocity_otg_y'],
                                                                                       boresight_az_angle)

        self.signals['velocity_otg_variance_x'], self.signals['velocity_otg_variance_y'], \
            self.signals['velocity_otg_covariance'] = rotate_2d_cov(self.signals['velocity_otg_variance_x'],
                                                                    self.signals['velocity_otg_variance_y'],
                                                                    self.signals['velocity_otg_covariance'],
                                                                    -boresight_az_angle)
        self.coordinate_system = 'SCS'

    def filter_single_radar(self, radar_id: int = 1):
        """
        Filter single radar data based on sensor ID
        :param radar_id:
        :return:
        """
        self.signals = self.signals[self.signals['sensor_id'] == radar_id]
        self.per_look = self.per_look[self.per_look['sensor_id'] == radar_id]
        self.per_sensor = self.per_sensor[self.per_sensor['sensor_id'] == radar_id]
