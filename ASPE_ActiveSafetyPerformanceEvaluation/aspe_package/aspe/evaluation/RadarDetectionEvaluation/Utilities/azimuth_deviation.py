from typing import Tuple, Union

import numpy as np
import pandas as pd

from aspe.utilities.MathFunctions import normalize_angle_vector


def calc_azimuth_for_given_rr_and_vel(range_rate: Union[float, np.ndarray, pd.Series],
                                      vx: Union[float, np.ndarray, pd.Series],
                                      vy: Union[float, np.ndarray, pd.Series]) \
        -> Tuple[Union[float, np.ndarray, pd.Series], Union[float, np.ndarray, pd.Series]]:
    """
    Calculation of azimuth for given range rate and velocity (x and y)
    :param range_rate: range rate for whic azimuth should be calculated
    :type range_rate: Union[float, np.ndarray, pd.Series]
    :param vx: velocity in x direction
    :type vx: Union[float, np.ndarray, pd.Series]
    :param vy: velocity in y direction
    :type vy: Union[float, np.ndarray, pd.Series]
    :return: tuple(azimuth_pos, azimuth_neg) - azimuth is ambiguous since it is based on arc-cos so two angles are
                                               returned

        Note:
            azimuth_pos > azimuth_neg
            NaN will be returned in case of:
            - velocity magnitude is zero
            - range rate is bigger than velocity magnitude
    """
    magnitude = np.hypot(vx, vy)
    heading = np.arctan2(vy, vx)

    azimuth_pos = normalize_angle_vector(np.arccos(range_rate / magnitude) + heading)
    azimuth_neg = normalize_angle_vector(-np.arccos(range_rate / magnitude) + heading)
    return azimuth_pos, azimuth_neg


def calc_azimuth_deviation_for_given_rr_and_vel(azimuth: Union[float, np.ndarray, pd.Series],
                                                range_rate: Union[float, np.ndarray, pd.Series],
                                                vx: Union[float, np.ndarray, pd.Series],
                                                vy: Union[float, np.ndarray, pd.Series]) \
        -> Union[float, np.ndarray]:
    """
    Calculation of azimuth deviation for given azimuth, range rate and velocity (x and y)
    :param azimuth: range rate for whic azimuth should be calculated
    :type azimuth: Union[float, np.ndarray, pd.Series]
    :param range_rate: range rate for whic azimuth should be calculated
    :type range_rate: Union[float, np.ndarray, pd.Series]
    :param vx: velocity in x direction
    :type vx: Union[float, np.ndarray, pd.Series]
    :param vy: velocity in y direction
    :type vy: Union[float, np.ndarray, pd.Series]
    :return: azimuth_deviation - deviation of azimuth angle for given parameters

        Note:
            NaN will be returned in case of:
            - velocity magnitude is zero
            - range rate is bigger than velocity magnitude

            For Series as input - ndarray array will be returned
    """
    azimuth_pos, azimuth_neg = calc_azimuth_for_given_rr_and_vel(range_rate, vx, vy)
    deviations_hyp = normalize_angle_vector(np.array([azimuth - azimuth_pos, azimuth - azimuth_neg]))
    abs_deviations_hyp = np.abs(deviations_hyp)
    ids = np.argmin(abs_deviations_hyp, axis=0)
    if len(deviations_hyp.shape) == 1:
        deviations = deviations_hyp[ids]
    else:
        index_axis_1 = np.arange(deviations_hyp.shape[1])
        deviations = deviations_hyp[(ids, index_axis_1)]
    return deviations

