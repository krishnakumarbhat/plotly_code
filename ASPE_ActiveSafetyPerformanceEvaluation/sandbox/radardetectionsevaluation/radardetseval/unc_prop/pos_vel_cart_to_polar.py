import numpy as np


def unc_prop_pos_rel_vel_cart_to_polar(x, y, vx_rel, vy_rel, cart_cov):
    """

    :param x:
    :param y:
    :param vx_rel:
    :param vy_rel:
    :param cart_cov:
    :return:
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
