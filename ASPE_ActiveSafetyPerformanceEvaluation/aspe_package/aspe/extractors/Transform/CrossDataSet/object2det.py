from typing import Union

import numpy as np
import pandas as pd

from aspe.extractors.Transform.objects import calc_object_rel_state_at_ref_point


def calc_polar_state_on_ref_point(objects_signals_scs: Union[pd.Series, pd.DataFrame],
                                  sensor_signals_scs_synch: Union[pd.Series, pd.DataFrame],
                                  ref_point_x: Union[float, np.ndarray],
                                  ref_point_y: Union[float, np.ndarray]) \
        -> Union[pd.Series, pd.DataFrame]:
    """
    Transformation of object state to polar state (position and velocity)
    :param objects_signals_scs: Signals from IObject interface in SCS
    :type objects_signals_scs: Union[pd.Series, pd.DataFrame]
    :param sensor_signals_scs_synch: Signals from IRadarSensor interface in SCS synchronized in length with
           objects_signals_scs
    :type sensor_signals_scs_synch: Union[pd.Series, pd.DataFrame]
    :param ref_point_x:
    :type ref_point_x: Union[float, np.ndarray]
    :param ref_point_y:
    :type ref_point_y: Union[float, np.ndarray]
    :return: Union[pd.Series, pd.DataFrame] polar state with fields:
             .range
             .azimuth
             .range_rate
             .cross_radial_velocity
    """
    rel_state = calc_object_rel_state_at_ref_point(objects_signals_scs, sensor_signals_scs_synch,
                                                   ref_point_x, ref_point_y)
    polar_state = cart_rel_state_to_polar_state(rel_state)

    return polar_state


def cart_rel_state_to_polar_state(rel_state: Union[pd.Series, pd.DataFrame]) -> Union[pd.Series, pd.DataFrame]:
    """
    Transformation of relative state (position and velocity) to polar state
    :param rel_state: relative state DataFrame or Series with:
           .position_x
           .position_y
           .velocity_rel_x
           .velocity_rel_y
    :return: Union[pd.Series, pd.DataFrame] polar state with fields:
             .range
             .azimuth
             .range_rate
             .cross_radial_velocity
    """
    det_range = np.hypot(rel_state.position_x, rel_state.position_y)
    azimuth = np.arctan2(rel_state.position_y, rel_state.position_x)
    sin_az = np.sin(azimuth)
    cos_az = np.cos(azimuth)
    range_rate = cos_az * rel_state.velocity_rel_x + sin_az * rel_state.velocity_rel_y
    cross_radial_velocity = -sin_az * rel_state.velocity_rel_x + cos_az * rel_state.velocity_rel_y

    data = {'range': det_range,
            'azimuth': azimuth,
            'range_rate': range_rate,
            'cross_radial_velocity': cross_radial_velocity}
    if isinstance(rel_state, pd.DataFrame):
        polar_state = pd.DataFrame(data)
    else:
        polar_state = pd.Series(data)

    return polar_state
