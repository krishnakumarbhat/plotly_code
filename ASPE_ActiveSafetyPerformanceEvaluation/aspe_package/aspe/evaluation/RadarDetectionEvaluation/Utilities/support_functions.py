from typing import Dict, Tuple, Union

import numpy as np
import pandas as pd

from aspe.extractors.Transform.CrossDataSet.object2det import cart_rel_state_to_polar_state
from aspe.extractors.Transform.velocity_cov import otg_to_rel_vel_cov


def deviation_cart_vs_polar(cart_state: Union[pd.Series, pd.DataFrame],
                            detection_state: Union[pd.Series, pd.DataFrame])\
        -> np.ndarray:
    """
    :param cart_state: relative state DataFrame or Series with:
           .position_x
           .position_y
           .velocity_rel_x
           .velocity_rel_y
    :type cart_state: Union[pd.Series, pd.DataFrame]
    :param detection_state: polar state with fields:
             .range
             .azimuth
             .range_rate
    :type detection_state: Union[pd.Series, pd.DataFrame]
    :return: np.ndarray, 3xN deviation with with columns:
                         0 - range
                         1 - azimuth
                         2 - range rate
    """
    ref_polar_state = cart_rel_state_to_polar_state(cart_state)
    deviation = np.array([ref_polar_state.range - detection_state.range,
                          ref_polar_state.azimuth - detection_state.azimuth,
                          ref_polar_state.range_rate - detection_state.range_rate])
    return deviation


def get_object_pos_rel_vel_cov(single_obj: pd.Series, sensor_motion: pd.Series) -> np.ndarray:
    """
    Get 4x4 covariance matrix for single object representing relative state (position and velocity)
    Relative velocity covariance will be calculated based on object over-the-ground velocity covariance and
    sensor motion uncertainties
    :param single_obj: object series with at least valid fields:
                       ['velocity_otg_variance_x']
                       ['velocity_otg_variance_y']
                       ['velocity_otg_covariance']
                       ['position_x']
                       ['position_y']
                       ['position_variance_x']
                       ['position_variance_y']
                       ['position_covariance']
    :param sensor_motion: sensor motion series with at least:
                          ['velocity_otg_variance_x']
                          ['velocity_otg_variance_y']
                          ['velocity_otg_covariance']
                          ['yaw_rate_variance']
    :return: np.ndarray 4x4 covaraince matrix with signals for
                        0 - position_x
                        1 - position_y
                        2 - velocity_rel_x
                        3 - velocity_rel_y
    """
    velocity_rel_variance_x, velocity_rel_variance_y, velocity_rel_covariance, = \
        otg_to_rel_vel_cov(single_obj.velocity_otg_variance_x, single_obj.velocity_otg_variance_y,
                           single_obj.velocity_otg_covariance, sensor_motion.velocity_otg_variance_x,
                           sensor_motion.velocity_otg_variance_y, sensor_motion.velocity_otg_covariance,
                           sensor_motion.yaw_rate_variance, single_obj.position_x, single_obj.position_y)
    cov = np.zeros([4, 4])
    cov[0, 0] = single_obj.position_variance_x
    cov[1, 1] = single_obj.position_variance_y
    cov[1, 0] = single_obj.position_covariance
    cov[0, 1] = single_obj.position_covariance
    cov[2, 2] = velocity_rel_variance_x
    cov[3, 3] = velocity_rel_variance_y
    cov[2, 3] = velocity_rel_covariance
    cov[3, 2] = velocity_rel_covariance
    return cov


def dict_based_df_concat(dict_of_dict_of_df: Dict, added_key_name: str):
    """
    Perform concat data frame concat based on nested dict.
    This is a function for flattening data
    :param dict_of_dict_of_df: nested dict where the structure is
           dict.dict.DataFrame
    :param added_key_name: key name to be added to flattened DataFrame
    :return: dict of DataFrames
    """

    flatten_dict_of_df = dict()
    dict_of_df = next(iter(dict_of_dict_of_df.values()))

    # Initialize output by list
    for key in dict_of_df.keys():
        flatten_dict_of_df[key] = list()

    # Assign key from first dict keys and accumulate data-frames into list
    for first_dict_key, single_output in dict_of_dict_of_df.items():
        for output_key, df in single_output.items():
            df[added_key_name] = first_dict_key
            flatten_dict_of_df[output_key].append(df)

    # Execute flattening by concat
    for key, df_list in flatten_dict_of_df.items():
        flatten_dict_of_df[key] = pd.concat(df_list, ignore_index=True)

    # Add as reference data frame with first dict keys
    flatten_dict_of_df[added_key_name] = pd.DataFrame(dict_of_dict_of_df.keys(), columns=[added_key_name])

    return flatten_dict_of_df


