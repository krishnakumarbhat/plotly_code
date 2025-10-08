import numpy as np


def unc_prop_pos_rel_vel_cart_to_polar(x: float, y: float, vx_rel: float, vy_rel: float,
                                       cart_cov: np.ndarray) -> np.ndarray:
    """

    :param x: position x in cartesian coordinate system
    :type x: float
    :param y: position y in cartesian coordinate system
    :type y: float
    :param vx_rel: relative velocity x in cartesian coordinate system
    :type vx_rel: float
    :param vy_rel: relative velocity y in cartesian coordinate system
    :type vy_rel: float
    :param cart_cov: covariance for corresponding state 4x4
    :type cart_cov: np.ndarray
    :return: np.ndarray 3x3 polar covariance matrix with indexes:
                            0 - range
                            1 - azimuth
                            2 - range rate
    """
    det_range = np.hypot(x, y)
    det_range_square = det_range * det_range
    det_range_cubic = det_range_square * det_range
    # Calculate Jacobian
    jacobian = np.zeros([3, 4])

    jacobian[0, 0] = x / det_range  # d_range / d_x
    jacobian[0, 1] = y / det_range  # d_range / d_y
    jacobian[1, 0] = -y / det_range_square  # d_azimuth / d_x
    jacobian[1, 1] = x / det_range_square  # d_azimuth / d_y
    jacobian[2, 0] = y * y / det_range_cubic * vx_rel - x * y / det_range_cubic * vy_rel  # d_range_rate / d_x
    jacobian[2, 1] = - x * y / det_range_cubic * vx_rel + x * x / det_range_cubic * vy_rel  # d_range_rate / d_y
    jacobian[2, 2] = x / det_range  # d_range_rate / d_vx
    jacobian[2, 3] = y / det_range  # d_range_rate / d_vy

    pol_cov = (jacobian @ cart_cov) @ jacobian.transpose()
    return pol_cov
