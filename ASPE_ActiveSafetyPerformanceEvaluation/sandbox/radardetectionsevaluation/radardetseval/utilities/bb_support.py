import numpy as np
from AptivDataExtractors.utilities.MathFunctions import rot_2d_sae_cs


def is_point_in_bb(point_x: np.array, point_y: np.array,
                   bb_center_x: np.array, bb_center_y: np.array,
                   bb_length: np.array, bb_width: np.array, bb_orientation: np.array):
    bb_length_half = bb_length / 2.0
    bb_width_half = bb_width / 2.0

    x_rel = point_x - bb_center_x
    y_rel = point_y - bb_center_y
    x_tcs, y_tcs = rot_2d_sae_cs(x_rel, y_rel, bb_orientation)
    f_valid_x = np.logical_and(x_tcs >= -bb_length_half, x_tcs <= bb_length_half)
    f_valid_y = np.logical_and(y_tcs >= -bb_width_half, y_tcs <= bb_width_half)
    f_valid = np.logical_and(f_valid_x, f_valid_y)
    return f_valid

