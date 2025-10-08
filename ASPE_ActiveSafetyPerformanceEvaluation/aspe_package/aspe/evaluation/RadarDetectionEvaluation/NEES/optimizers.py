import numpy as np
import pandas as pd
from scipy import optimize

from aspe.evaluation.RadarDetectionEvaluation.NEES.nees_custom_functions import (
    nees_3d_components,
    nees_3d_components_with_ref_cov,
)
from aspe.evaluation.RadarDetectionEvaluation.NEES.nees_objective_functions import (
    nees_3d_constant_ref_cov,
    nees_3d_no_ref_cov,
)
from aspe.evaluation.RadarDetectionEvaluation.Utilities.support_functions import (
    deviation_cart_vs_polar,
    get_object_pos_rel_vel_cov,
)
from aspe.extractors.Transform.objects import calc_object_rel_state_at_ref_point


def nearest_det_3d_nees_constant_obj_cov(radar_object, sensor_synch, detection):
    """
    Finding ref point on object for which minimum NEES metric is observed for given detection.
    Constant reference covariance matrix is assumed (for each point on reference object).
    :param radar_object:
    :param sensor_synch:
    :param detection:
    :return: output structure from scipy.optimize.minimize
    """
    ref_cov = get_object_pos_rel_vel_cov(radar_object, sensor_synch)
    det_state = detection[['range', 'azimuth', 'range_rate']]
    det_cov = np.diag([detection['range_variance'], detection['azimuth_variance'], detection['range_rate_variance']])

    initial_ref_point = np.array([0.5, 0.5])
    bounds = ((0.0, 1.0), (0.0, 1.0))
    opt_out = optimize.minimize(nees_3d_constant_ref_cov,
                                x0=initial_ref_point,
                                args=(radar_object, sensor_synch, ref_cov, det_state, det_cov),
                                bounds=bounds)

    ref_state = calc_object_rel_state_at_ref_point(radar_object, sensor_synch, opt_out.x[0], opt_out.x[1])
    deviation = deviation_cart_vs_polar(ref_state, det_state)
    nees = nees_3d_components_with_ref_cov(ref_state, ref_cov, det_state, det_cov)
    output = get_optimization_outputs(opt_out, ref_state, deviation, nees)

    return output


def nearest_det_3d_nees_no_obj_cov(radar_object, sensor_synch, detection):
    """
    Faster version of finding ref point on object for which minimum NEES metric is observed for given detection.
    Constant reference covariance matrix is assumed.
    :param radar_object:
    :param sensor_synch:
    :param detection:
    :return: output structure from scipy.optimize.minimize
    """
    det_state = detection[['range', 'azimuth', 'range_rate']]
    det_variances = np.array(
        [detection['range_variance'], detection['azimuth_variance'], detection['range_rate_variance']])

    initial_ref_point = np.array([0.5, 0.5])
    bounds = ((0.0, 1.0), (0.0, 1.0))
    opt_out = optimize.minimize(nees_3d_no_ref_cov,
                                x0=initial_ref_point,
                                args=(radar_object, sensor_synch, det_state, det_variances),
                                bounds=bounds)
    ref_state = calc_object_rel_state_at_ref_point(radar_object, sensor_synch, opt_out.x[0], opt_out.x[1])
    deviation = deviation_cart_vs_polar(ref_state, det_state)
    nees = nees_3d_components(ref_state, det_state, np.diag(det_variances))
    output = get_optimization_outputs(opt_out, ref_state, deviation, nees)

    return output


def get_optimization_outputs(opt_out, ref_state, deviation, nees):
    opt_out_series = pd.Series({
        'ref_point_x': opt_out.x[0],
        'ref_point_y': opt_out.x[1],
        'opt_success': opt_out.success,
        'opt_nit': opt_out.nit,
    })
    deviation_series = pd.Series({
        'deviation_range': deviation[0],
        'deviation_azimuth': deviation[1],
        'deviation_range_rate': deviation[2],
    })
    output = pd.concat([opt_out_series, ref_state, deviation_series, nees])
    return output
