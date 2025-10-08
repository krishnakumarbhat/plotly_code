from copy import deepcopy

import numpy as np

from aspe.extractors.Interfaces.IHost import IHost
from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors
from aspe.extractors.Transform.velocity_cov import yaw_rate_influence_cov_transformation
from aspe.utilities.data_filters import slice_df_based_on_other
from aspe.utilities.MathFunctions import calc_velocity_in_position


def host2sensor_motion(sensor_ds_in: IRadarSensors, host_ds: IHost, inplace=False):
    """
    Signals to be modified:
    signals.velocity_otg_x
    signals.velocity_otg_y
    signals.velocity_otg_variance_x
    signals.velocity_otg_variance_y
    signals.velocity_otg_covariance
    signals.yaw_rate
    signals.yaw_rate_variance

    :param sensor_ds_in: Input sensor data det
    :param host_ds: Input host data det - they need to be scan index synchronized
    :param inplace: flag indicating if input data set should be modified or not
    :return:
    """
    if inplace:
        sensor_ds_out = sensor_ds_in
    else:
        sensor_ds_out = deepcopy(sensor_ds_in)

    sensor_signals = sensor_ds_in.signals
    host_signal_synch = slice_df_based_on_other(host_ds.signals, sensor_signals, ['scan_index'], ['scan_index'])
    sensor_mounting_synch = slice_df_based_on_other(sensor_ds_in.per_sensor,
                                                    sensor_signals, ['sensor_id'], ['sensor_id'])

    sensor_ds_out.signals['velocity_otg_x'], sensor_ds_out.signals['velocity_otg_y'] = \
        calc_velocity_in_position(np.array([0.0]), np.array([0.0]),
                                  host_signal_synch['velocity_otg_x'], host_signal_synch['velocity_otg_y'],
                                  host_signal_synch['yaw_rate'],
                                  sensor_mounting_synch['position_x'], sensor_mounting_synch['position_y'])

    yr_inf_variance_x, yr_inf_variance_y, yr_inf_covariance = \
        yaw_rate_influence_cov_transformation(host_signal_synch['yaw_rate_variance'],
                                              sensor_mounting_synch['position_x'], sensor_mounting_synch['position_y'])

    sensor_ds_out.signals['velocity_otg_variance_x'] = host_signal_synch['velocity_otg_variance_x'] + yr_inf_variance_x
    sensor_ds_out.signals['velocity_otg_variance_y'] = host_signal_synch['velocity_otg_variance_y'] + yr_inf_variance_y
    sensor_ds_out.signals['velocity_otg_covariance'] = host_signal_synch['velocity_otg_covariance'] + yr_inf_covariance

    sensor_ds_out.signals['yaw_rate'] = host_signal_synch['yaw_rate']
    sensor_ds_out.signals['yaw_rate_variance'] = host_signal_synch['yaw_rate_variance']

    return sensor_ds_out
