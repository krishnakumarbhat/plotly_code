import warnings

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.Exceptions import WrongCoordinateSystem
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors
from aspe.extractors.Transform.CrossDataSet.object2det import calc_polar_state_on_ref_point
from aspe.utilities.data_filters import slice_df_based_on_other


def calc_object_fov_info(scs_objects_ds: IObjects, scs_sensors_ds: IRadarSensors) -> pd.DataFrame:
    """
    Calculate FoV information for object dependent on Radar Sensor information
    Note:
        sensors_ds should contain only single sensor information
        FoV information is calculated based on 8 reference points: 4 corners and 4 middle points - there may be
        slight mismatch with real spread of polar coordinates.
    :param scs_objects_ds: Object data set - expected in SCS
    :type scs_objects_ds: IObjects
    :param scs_sensors_ds: Sensors data set - expected in SCS
    :type scs_sensors_ds: IRadarSensors
    :return: pd.DataFrame with information:
             fov_info['f_any_ref_points_in_fov']
             fov_info['f_all_ref_points_in_fov']
             fov_info['min_range']
             fov_info['max_range']
             fov_info['range_spread']
             fov_info['min_azimuth']
             fov_info['max_azimuth']
             fov_info['azimuth_spread']
             fov_info['min_range_rate']
             fov_info['max_range_rate']
             fov_info['range_rate_spread']
    """
    obj_signals_scs = scs_objects_ds.signals
    if scs_sensors_ds.coordinate_system != 'SCS':
        raise WrongCoordinateSystem(f'Wrong coordinate system for sensor data, should be SCS, '
                                    f'but there is {scs_sensors_ds.coordinate_system }')
    if scs_objects_ds.coordinate_system != 'SCS':
        raise WrongCoordinateSystem(f'Wrong coordinate system for object data, should be SCS, '
                                    f'but there is {scs_objects_ds.coordinate_system }')

    sensor_ext_signals = scs_sensors_ds.signals.merge(scs_sensors_ds.per_look, on='look_id')
    # Synchronize with Objects
    sensor_ext_signals_synch = slice_df_based_on_other(sensor_ext_signals, obj_signals_scs,
                                                       ['scan_index'], ['scan_index'])

    ref_points = np.array([[0.0, 0.0],
                           [0.5, 0.0],
                           [1.0, 0.0],
                           [1.0, 0.5],
                           [1.0, 1.0],
                           [0.5, 1.0],
                           [0.0, 1.0],
                           [0.0, 0.5]])

    range_list = []
    azimuth_list = []
    range_rate_list = []
    for single_ref_point in ref_points:
        det_state = calc_polar_state_on_ref_point(obj_signals_scs,
                                                  sensor_ext_signals_synch,
                                                  single_ref_point[0],
                                                  single_ref_point[1])
        range_list.append(det_state.range)
        azimuth_list.append(det_state.azimuth)
        range_rate_list.append(det_state.range_rate)

    range_array = np.array(range_list)
    azimuth_array = np.array(azimuth_list)
    range_rate_array = np.array(range_rate_list)

    valid_range = range_array <= sensor_ext_signals_synch.max_range.values
    valid_azimuth = np.logical_and(azimuth_array <= sensor_ext_signals_synch.max_azimuth.values,
                                   azimuth_array >= sensor_ext_signals_synch.min_azimuth.values)

    f_in_fov = np.logical_and(valid_range, valid_azimuth)
    f_not_in_fov = np.logical_not(f_in_fov)
    range_array[f_not_in_fov] = np.nan
    azimuth_array[f_not_in_fov] = np.nan
    range_rate_array[f_not_in_fov] = np.nan

    fov_info = pd.DataFrame()
    # Caution - in multi-thread operations this will be undefined behavior
    with warnings.catch_warnings():
        warnings.filterwarnings('ignore', r'All-NaN slice encountered')  # It is expected
        fov_info['f_any_ref_points_in_fov'] = f_in_fov.any(axis=0)
        fov_info['f_all_ref_points_in_fov'] = f_in_fov.all(axis=0)
        fov_info['sensor_valid'] = sensor_ext_signals_synch['sensor_valid']
        fov_info['min_range'] = np.nanmin(range_array, axis=0)
        fov_info['max_range'] = np.nanmax(range_array, axis=0)
        fov_info['range_spread'] = fov_info['max_range'] - fov_info['min_range']
        fov_info['min_azimuth'] = np.nanmin(azimuth_array, axis=0)
        fov_info['max_azimuth'] = np.nanmax(azimuth_array, axis=0)
        fov_info['azimuth_spread'] = fov_info['max_azimuth'] - fov_info['min_azimuth']
        fov_info['min_range_rate'] = np.nanmin(range_rate_array, axis=0)
        fov_info['max_range_rate'] = np.nanmax(range_rate_array, axis=0)
        fov_info['range_rate_spread'] = fov_info['max_range_rate'] - fov_info['min_range_rate']
    return fov_info
