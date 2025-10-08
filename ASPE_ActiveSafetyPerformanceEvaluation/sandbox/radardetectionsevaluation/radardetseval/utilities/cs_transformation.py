from copy import deepcopy

import numpy as np
import pandas as pd
from AptivDataExtractors.utilities.MathFunctions import vcs2scs, rot_2d_sae_cs

from radardetseval.conversions.object_conversion import vcs2scs_sensor_velocity, rotate_2d_cov, \
    vcs2scs_sensor_velocity_cov
from radardetseval.utilities.data_filters import slice_df_based_on_other


def vcs2scs_object(objects_vcs, scs_location):
    object_signals = objects_vcs.signals

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
                         'center_y'
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
                                                     scs_location.boresight_az_angle)

    scs_vx_var, scs_vy_var, scs_vxvy_cov = rotate_2d_cov(object_signals['velocity_otg_variance_x'],
                                                         object_signals['velocity_otg_variance_y'],
                                                         object_signals['velocity_otg_covariance'],
                                                         scs_location.boresight_az_angle)


    # TODO: Normalize angle
    signals_scs['bounding_box_orientation'] = object_signals['bounding_box_orientation'] - \
        scs_location.boresight_az_angle

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

    objects_scs = deepcopy(objects_vcs)
    objects_scs.signals = signals_scs
    objects_scs.coordinate_system = 'SCS'
    objects_scs.coordinate_system_location = deepcopy(scs_location)

    return objects_scs


def get_reference_sensor_data(est_sensor, ref_host):
    sensor_signals = est_sensor.signals
    host_signal_synch = slice_df_based_on_other(ref_host.signals, sensor_signals, ['scan_index'], ['scan_index'])
    sensor_mounting_synch = slice_df_based_on_other(est_sensor.per_sensor, sensor_signals, ['sensor_id'], ['sensor_id'])
    columns_to_add = ['velocity_otg_x',
                      'velocity_otg_y',
                      'velocity_otg_variance_x',
                      'velocity_otg_variance_y',
                      'velocity_otg_covariance',
                      'yaw_rate',
                      'yaw_rate_variance']
    columns_to_keep = ['scan_index',
                       'timestamp',
                       'slot_id',
                       'unique_id',
                       'sensor_id',
                       'look_id',
                       'sensor_valid']
    columns = columns_to_keep + columns_to_add
    signals_scs = pd.DataFrame(columns=columns)
    signals_scs[columns_to_keep] = sensor_signals[columns_to_keep]

    scs_sensor_velocity_otg_x, scs_sensor_velocity_otg_y = \
        vcs2scs_sensor_velocity(host_signal_synch['velocity_otg_x'], host_signal_synch['velocity_otg_y'],
                                host_signal_synch['yaw_rate'], sensor_mounting_synch['position_x'],
                                sensor_mounting_synch['position_y'], sensor_mounting_synch['boresight_az_angle'])

    scs_sensor_velocity_otg_variance_x, scs_sensor_velocity_otg_variance_y, scs_sensor_velocity_otg_covariance =\
        vcs2scs_sensor_velocity_cov(host_signal_synch['velocity_otg_variance_x'],
                                    host_signal_synch['velocity_otg_variance_y'],
                                    host_signal_synch['velocity_otg_covariance'],
                                    host_signal_synch['yaw_rate_variance'], sensor_mounting_synch['position_x'],
                                    sensor_mounting_synch['position_y'], sensor_mounting_synch['boresight_az_angle'])

    signals_scs['velocity_otg_x'] = scs_sensor_velocity_otg_x
    signals_scs['velocity_otg_y'] = scs_sensor_velocity_otg_y
    signals_scs['velocity_otg_variance_x'] = scs_sensor_velocity_otg_variance_x
    signals_scs['velocity_otg_variance_y'] = scs_sensor_velocity_otg_variance_y
    signals_scs['velocity_otg_covariance'] = scs_sensor_velocity_otg_covariance
    signals_scs['yaw_rate'] = host_signal_synch['yaw_rate']
    signals_scs['yaw_rate_variance'] = host_signal_synch['yaw_rate_variance']

    ref_sensor = deepcopy(est_sensor)
    ref_sensor.signals = signals_scs
    return ref_sensor
