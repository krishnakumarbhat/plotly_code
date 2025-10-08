import numpy as np
import pandas as pd
import time

from radardetseval.association.objective_func.nees_cart_vs_polar import deviation_cart_vs_polar
from radardetseval.association.objective_func.nees_for_object import nees_between_object_and_det, \
    nees_between_object_and_det_fast
from radardetseval.configs.default_radar import DefaultRadar

from radardetseval.configs.defaut_reference import DefaultReference
from radardetseval.conversions.objects_into_polyline import get_object_state_for_given_ref_point, get_object_pos_vel_cov
from scipy import optimize
from radardetseval.simulators.DetFromRadarObjectSim import DetFromRadarObjectSim
from radardetseval.simulators.RadarObjectSim import RadarObjectSim


def find_nearest_det_on_object(radar_object, detection):
    """
    Finding ref point on object for which minimum NEES metric is observed for given detection.
    It uses simple and user convenient objective function which takes radar object and detection to
    calculate NEES value.
    :param radar_object:
    :param detection:
    :return:
    """
    initial_ref_point = np.array([0.5, 0.5])
    bounds = ((0.0, 1.0), (0.0, 1.0))
    opt_out = optimize.minimize(nees_between_object_and_det,
                                x0=initial_ref_point,
                                args=(radar_object, detection),
                                bounds=bounds)
    ref_state = get_object_state_for_given_ref_point(radar_object, opt_out.x)
    det_state = np.array([detection.det_range, detection.det_azimuth, detection.det_range_rate])
    deviation = deviation_cart_vs_polar(ref_state, det_state)
    output = get_optimization_outputs(opt_out, ref_state, deviation)
    return output


def find_nearest_det_on_object_fast(radar_object, detection):
    """
    Faster version of finding ref point on object for which minimum NEES metric is observed for given detection.
    Constant reference covariance matrix is assumed.
    :param radar_object:
    :param detection:
    :return:
    """
    ref_cov = get_object_pos_otg_vel_cov(radar_object)
    det_state = np.array([detection.det_range, detection.det_azimuth, detection.det_range_rate])
    det_cov = np.diag([detection.det_range_var, detection.det_azimuth_var, detection.det_range_rate_var])

    initial_ref_point = np.array([0.5, 0.5])
    bounds = ((0.0, 1.0), (0.0, 1.0))
    opt_out = optimize.minimize(nees_between_object_and_det_fast,
                                x0=initial_ref_point,
                                args=(radar_object, ref_cov, det_state, det_cov),
                                bounds=bounds)
    ref_state = get_object_state_for_given_ref_point(radar_object, opt_out.x)
    deviation = deviation_cart_vs_polar(ref_state, det_state)
    output = get_optimization_outputs(opt_out, ref_state, deviation)
    return output


def get_optimization_outputs(opt_out, ref_state, deviation):
    output = pd.Series({
        'ref_point_x': opt_out.x[0],
        'ref_point_y': opt_out.x[1],
        'nees_value': opt_out.fun,
        'opt_success': opt_out.success,
        'opt_nit': opt_out.nit,
        'ref_position_x': ref_state[0],
        'ref_position_y': ref_state[1],
        'ref_velocity_rel_x': ref_state[2],
        'ref_velocity_rel_y': ref_state[3],
        'deviation_range': deviation[0],
        'deviation_azimuth': deviation[1],
        'deviation_range_rate': deviation[2]
    })
    return output


def get_object_pos_otg_vel_cov(single_obj):
    cov = np.zeros([4, 4])
    cov[0, 0] = single_obj.position_variance_x
    cov[1, 1] = single_obj.position_variance_y
    cov[1, 0] = single_obj.position_covariance
    cov[0, 1] = single_obj.position_covariance
    cov[2, 2] = single_obj.velocity_otg_variance_x
    cov[3, 3] = single_obj.velocity_otg_variance_y
    cov[2, 3] = single_obj.velocity_otg_covariance
    cov[3, 2] = single_obj.velocity_otg_covariance
    return cov


if __name__ == '__main__':
    single_row_dict = {
            'id': 0,
            'position_x': 6.0,
            'position_y': 5.0,
            'center_x': 6.0,
            'center_y': 5.0,
            'velocity_rel_x': 4.0,
            'velocity_rel_y': 0.0,
            'bounding_box_dimensions_x': 4.0,
            'bounding_box_dimensions_y': 2.0,
            'bounding_box_orientation': np.deg2rad(0.0),
            'bounding_box_refpoint_long_offset_ratio': 0.5,
            'bounding_box_refpoint_lat_offset_ratio': 0.5,
            'yaw_rate': np.deg2rad(0.0),  # [rad/s]
    }
    ref_pos_std = DefaultReference.pos_std
    ref_vel_std = DefaultReference.vel_std
    pos_cov = np.power(np.diag([ref_pos_std, ref_pos_std]), 2.0)
    vel_cov = np.power(np.diag([ref_vel_std, ref_vel_std]), 2.0)
    meas_cov = np.power(np.diag([DefaultRadar.range_std, DefaultRadar.azimuth_std, DefaultRadar.range_rate_std]),
                        2.0)
    single_obj_series = pd.Series(single_row_dict)
    det_simulator = DetFromRadarObjectSim(single_obj_series)
    object_simulator = RadarObjectSim(single_obj_series)
    radar_obj = object_simulator.sim(pos_cov, vel_cov)
    det = det_simulator.sim_single_point_from_ref_point(meas_cov, 0.0, 0.5)

    t1 = time.time()
    opt_output = find_nearest_det_on_object(radar_obj, det)
    t2 = time.time()
    opt_output_fast = find_nearest_det_on_object_fast(radar_obj, det)
    t3 = time.time()

    print(opt_output)
    print(opt_output_fast)
    print(f'find_nearest_det_on_object: {t2 - t1} s')
    print(f'find_nearest_det_on_object: {t3 - t2} s')


