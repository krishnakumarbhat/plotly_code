import numpy as np
import pandas as pd

from aspe.evaluation.RadarDetectionEvaluation.NEES.nees import nees_value
from aspe.evaluation.RadarDetectionEvaluation.Utilities.support_functions import deviation_cart_vs_polar
from aspe.evaluation.RadarDetectionEvaluation.Utilities.unc_prop import unc_prop_pos_rel_vel_cart_to_polar


def nees_cart_vs_polar_pos_vel(cart_state: pd.Series, cart_cov: np.ndarray,
                               polar_state: pd.Series, polar_cov: np.ndarray) \
        -> float:
    """
    Calculating normalized estimated error squared (NEES) between polar and cartesian state with corresponding
    covariance matrices.
    NEES is calculated in polar domain so cartesian state is transform to polar.
    :param cart_state: Series with at least 4 fields:
                       cart_state.position_x
                       cart_state.position_y
                       cart_state.velocity_rel_x
                       cart_state.velocity_rel_y

    :param cart_cov: 4x4 covariance matrix corresponding to state (with the same order)
    :param polar_state: Series with at least 4 fields:
                        polar_state.range
                        polar_state.azimuth
                        polar_state.range_rate
    :param polar_cov: 3x3 covariance matrix corresponding to state (with the same order)
    :return: float, single nees value
    """
    ref_cov_polar = unc_prop_pos_rel_vel_cart_to_polar(cart_state.position_x,
                                                       cart_state.position_y,
                                                       cart_state.velocity_rel_x,
                                                       cart_state.velocity_rel_y,
                                                       cart_cov)

    cov_sum = ref_cov_polar + polar_cov

    deviation = deviation_cart_vs_polar(cart_state, polar_state)

    nees = nees_value(deviation, cov_sum)
    return nees


def nees_3d_components_with_ref_cov(cart_state: pd.Series, cart_cov: np.ndarray,
                                    polar_state: pd.Series, polar_cov: np.ndarray) \
        -> pd.Series:
    """
    Calculating normalized estimated error squared (NEES) between polar and cartesian state with corresponding
    covariance matrices.
    Function calculates different components of NEES
    NEES is calculated in polar domain so cartesian state is transform to polar.
    :param cart_state: Series with at least 4 fields:
                       cart_state.position_x
                       cart_state.position_y
                       cart_state.velocity_rel_x
                       cart_state.velocity_rel_y

    :param cart_cov: 4x4 covariance matrix corresponding to state (with the same order)
    :param polar_state: Series with at least 4 fields:
                        polar_state.range
                        polar_state.azimuth
                        polar_state.range_rate
    :param polar_cov: 3x3 covariance matrix corresponding to state (with the same order)
    :return: pd.Series, see 'nees_3d_components' outputs
    """
    ref_cov_polar = unc_prop_pos_rel_vel_cart_to_polar(cart_state.position_x,
                                                       cart_state.position_y,
                                                       cart_state.velocity_rel_x,
                                                       cart_state.velocity_rel_y,
                                                       cart_cov)

    cov_sum = ref_cov_polar + polar_cov
    nees = nees_3d_components(cart_state, polar_state, cov_sum)

    return nees


def nees_3d_components(cart_state: pd.Series, polar_state: pd.Series, cov_sum: np.ndarray) -> pd.Series:
    """
    Calculating normalized estimated error squared (NEES) between polar and cartesian state with corresponding
    covariance matrices.
    Function calculates different components of NEES
    NEES is calculated in polar domain so cartesian state is transform to polar.
    :param cart_state: Series with at least 4 fields:
                       cart_state.position_x
                       cart_state.position_y
                       cart_state.velocity_rel_x
                       cart_state.velocity_rel_y
    :param polar_state: Series with at least 4 fields:
                        polar_state.range
                        polar_state.azimuth
                        polar_state.range_rate
    :param cov_sum: 3x3 covariance matrix corresponding to polar state (with the same order)
    :return: pd.Series with:
             ['nees_3d']
             ['nees_range']
             ['nees_azimuth']
             ['nees_range_rate']
             ['nees_azimuth_range_rate']
    """
    deviation = deviation_cart_vs_polar(cart_state, polar_state)
    nees_azimuth_range_rate = nees_value(deviation[1:], cov_sum[1:, 1:])
    nees_range = deviation[0] ** 2 / cov_sum[0, 0]
    nees_azimuth = deviation[1] ** 2 / cov_sum[1, 1]
    nees_range_rate = deviation[2] ** 2 / cov_sum[2, 2]

    nees_3d = nees_value(deviation, cov_sum)
    nees = pd.Series({
        'nees_3d': nees_3d,
        'nees_range': nees_range,
        'nees_azimuth': nees_azimuth,
        'nees_range_rate': nees_range_rate,
        'nees_azimuth_range_rate': nees_azimuth_range_rate,
    })

    return nees

