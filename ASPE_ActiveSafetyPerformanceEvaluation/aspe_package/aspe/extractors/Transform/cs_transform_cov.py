from typing import Union

import numpy as np

from aspe.utilities.UncertaintyPropagation import propagate_uncertainty_2d


def rotate_2d_cov(x_var: np.array, y_var: np.array, xy_cov: np.array, rotation_angle: Union[float, np.array]):
    """

    :param x_var:
    :param y_var:
    :param xy_cov:
    :param rotation_angle:
    :return:
    """
    if isinstance(rotation_angle, float):
        j_11 = np.full_like(x_var, np.cos(rotation_angle))
        j_21 = np.full_like(x_var, np.sin(rotation_angle))
    else:
        j_11 = np.cos(rotation_angle)
        j_21 = np.sin(rotation_angle)

    j_12 = -j_21
    j_22 = j_11

    x_var_rot, y_var_rot, xy_cov_rot = propagate_uncertainty_2d(x_var, y_var, xy_cov, j_11, j_12, j_21, j_22)
    return x_var_rot, y_var_rot, xy_cov_rot
