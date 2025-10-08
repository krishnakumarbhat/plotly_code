import numpy as np
from radardetseval.association.objective_func.nees_cart_vs_polar import nees_cart_vs_polar
from radardetseval.conversions.objects_into_polyline import get_object_state_for_given_ref_point, get_object_pos_vel_cov


def nees_between_object_and_det(ref_point, radar_object, detection):

    ref_state = get_object_state_for_given_ref_point(radar_object, ref_point)
    ref_cov = get_object_pos_vel_cov(radar_object)

    det_state = np.array([detection.det_range, detection.det_azimuth, detection.det_range_rate])
    det_cov = np.diag([detection.det_range_var, detection.det_azimuth_var, detection.det_range_rate_var])

    nees = nees_cart_vs_polar(ref_state, ref_cov, det_state, det_cov)
    return nees


def nees_between_object_and_det_fast(ref_point, radar_object, ref_cov, det_state, det_cov):

    ref_state = get_object_state_for_given_ref_point(radar_object, ref_point)

    nees = nees_cart_vs_polar(ref_state, ref_cov, det_state, det_cov)
    return nees
