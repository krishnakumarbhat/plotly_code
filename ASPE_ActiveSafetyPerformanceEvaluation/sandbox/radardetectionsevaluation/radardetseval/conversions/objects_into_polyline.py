import numpy as np
import pandas as pd
from AptivDataExtractors.utilities.MathFunctions import calc_velocity_in_position, calc_position_in_bounding_box
from radardetseval.Iface.PolyLineDS import PolyLineDS


def object_into_polyline(single_obj):
    ref_points = np.array([[0.0, 0.0],
                           [1.0, 0.0],
                           [1.0, 1.0],
                           [0.0, 1.0],
                           [0.0, 0.0]])

    poly_line = PolyLineDS()

    for single_ref_point in ref_points:
        state = get_object_state_for_given_ref_point(single_obj, single_ref_point)

        cov = get_object_pos_vel_cov(single_obj)

        poly_line.add_point(state[0], state[1], state[2], state[3], cov)
    return poly_line


def get_object_state_for_given_ref_point(single_obj, ref_point):
    pos_x, pos_y = calc_position_in_bounding_box(single_obj.position_x, single_obj.position_y,
                                                 single_obj.bounding_box_dimensions_x,
                                                 single_obj.bounding_box_dimensions_y,
                                                 single_obj.bounding_box_orientation,
                                                 single_obj.bounding_box_refpoint_long_offset_ratio,
                                                 single_obj.bounding_box_refpoint_lat_offset_ratio,
                                                 ref_point[0], ref_point[1])
    vel_x, vel_y = calc_velocity_in_position(single_obj.position_x, single_obj.position_y,
                                             single_obj.velocity_otg_x, single_obj.velocity_otg_y,
                                             single_obj.yaw_rate,
                                             pos_x, pos_y)

    state = np.array([pos_x, pos_y, vel_x, vel_y])
    return state


def get_object_pos_vel_cov(single_obj):
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
        'position_variance_x': 1.0,
        'position_variance_y': 2.0,
        'position_covariance': 0.4,
        'velocity_otg_x': 4.0,
        'velocity_otg_y': 0.0,
        'velocity_otg_variance_x': 0.1,
        'velocity_otg_variance_y': 0.4,
        'velocity_otg_covariance': 0.05,
        'bounding_box_dimensions_x': 4.0,
        'bounding_box_dimensions_y': 2.0,
        'bounding_box_orientation': np.deg2rad(0.0),
        'bounding_box_refpoint_long_offset_ratio': 0.5,
        'bounding_box_refpoint_lat_offset_ratio': 0.5,
        'yaw_rate': np.deg2rad(0.0),  # [rad/s]
    }
    single_obj_series = pd.Series(single_row_dict)
    rectangle = object_into_polyline(single_obj_series)
