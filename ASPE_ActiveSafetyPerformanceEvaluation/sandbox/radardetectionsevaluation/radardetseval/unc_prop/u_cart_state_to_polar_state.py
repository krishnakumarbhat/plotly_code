from uncertainties import ufloat
from uncertainties import umath


def u_cart_state_to_polar_state(x: ufloat, y: ufloat, vx: ufloat, vy: ufloat):
    """
    Cartesian to polar transformation -> version for handling uncertain inputs
    :param x:
    :param y:
    :param vx:
    :param vy:
    :return:
    """
    range = umath.hypot(x, y)
    azimuth = umath.atan2(y, x)
    sin_az = umath.sin(azimuth)
    cos_az = umath.cos(azimuth)
    range_rate = cos_az * vx + sin_az * vy
    cross_radial_vel = -sin_az * vx + cos_az * vy
    return range, azimuth, range_rate, cross_radial_vel
