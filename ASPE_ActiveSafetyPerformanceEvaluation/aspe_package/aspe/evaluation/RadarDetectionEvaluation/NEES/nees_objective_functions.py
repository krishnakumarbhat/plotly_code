import numpy as np
import pandas as pd

from aspe.evaluation.RadarDetectionEvaluation.NEES.nees_custom_functions import nees_cart_vs_polar_pos_vel
from aspe.evaluation.RadarDetectionEvaluation.Utilities.support_functions import deviation_cart_vs_polar
from aspe.extractors.Transform.objects import calc_object_rel_state_at_ref_point


def nees_3d_constant_ref_cov(ref_point: np.ndarray,
                             object_state: pd.Series,
                             sensor_motion: pd.Series,
                             ref_cov: np.ndarray,
                             det_state: pd.Series,
                             det_cov: np.ndarray) \
        -> float:
    """
    3D Nees value calculation for constant reference covariance
    :param ref_point: two-elements np.ndarray,
                      ref_point[0] - ref_point_x
                      ref_point[1] - ref_point_y
    :param object_state: object series with relevant information to transform to relative frame at given point
    :param sensor_motion: sensor motion signals
    :param ref_cov: 4x4 reference covariance for:
                    cart_state.position_x
                    cart_state.position_y
                    cart_state.velocity_rel_x
                    cart_state.velocity_rel_y
    :param det_state: Series with at least 4 fields:
                      polar_state.range
                      polar_state.azimuth
                      polar_state.range_rate
    :param det_cov: 3x3 covariance matrix corresponding to state (with the same order)
    :return: float, single NEES value
    """
    ref_state = calc_object_rel_state_at_ref_point(object_state, sensor_motion, ref_point[0], ref_point[1])
    nees = nees_cart_vs_polar_pos_vel(ref_state, ref_cov, det_state, det_cov)
    return nees


def nees_3d_no_ref_cov(ref_point: np.ndarray,
                       object_state: pd.Series,
                       sensor_motion: pd.Series,
                       det_state: pd.Series,
                       det_variances: np.ndarray) \
        -> float:
    """
    3D Nees value calculation for no reference covariance
    :param ref_point: two-elements np.ndarray,
                      ref_point[0] - ref_point_x
                      ref_point[1] - ref_point_y
    :param object_state: object series with relevant information to transform to relative frame at given point
    :param sensor_motion: sensor motion signals
    :param det_state: Series with at least 4 fields:
                      polar_state.range
                      polar_state.azimuth
                      polar_state.range_rate
    :param det_variances: 3-elements variance array corresponding to state (with the same order)
    :return: float, single NEES value
    """
    ref_state = calc_object_rel_state_at_ref_point(object_state, sensor_motion, ref_point[0], ref_point[1])
    deviation = deviation_cart_vs_polar(ref_state, det_state)
    nees = np.sum(deviation ** 2 / det_variances)
    return nees
