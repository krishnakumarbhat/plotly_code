import numpy as np

from aspe.utilities.MathFunctions import rot_2d_sae_cs


def is_point_in_bb(point_x: np.array, point_y: np.array,
                   bb_center_x: np.array, bb_center_y: np.array,
                   bb_length: np.array, bb_width: np.array, bb_orientation: np.array):
    """
    Check if points are in given bounding box
    :param point_x: x coordinate of examined point
    :type point_x: np.array
    :param point_y: y coordinate of examined point
    :type point_y: np.array
    :param bb_center_x: x coordinate of bounding box center
    :type bb_center_x: np.array
    :param bb_center_y: y coordinate of bounding box center
    :type bb_center_y: np.array
    :param bb_length: length of bounding box
    :type bb_length: np.array
    :param bb_width: length of bounding box
    :type bb_width: np.array
    :param bb_orientation: orientation of bounding box (yaw angle)
    :type bb_orientation: np.array
    :return: bool np.array - indication if given point is inside defined bounding box
    """
    bb_length_half = bb_length / 2.0
    bb_width_half = bb_width / 2.0

    x_rel = point_x - bb_center_x
    y_rel = point_y - bb_center_y
    x_tcs, y_tcs = rot_2d_sae_cs(x_rel, y_rel, bb_orientation)
    f_valid_x = np.logical_and(x_tcs >= -bb_length_half, x_tcs <= bb_length_half)
    f_valid_y = np.logical_and(y_tcs >= -bb_width_half, y_tcs <= bb_width_half)
    f_valid = np.logical_and(f_valid_x, f_valid_y)
    return f_valid
