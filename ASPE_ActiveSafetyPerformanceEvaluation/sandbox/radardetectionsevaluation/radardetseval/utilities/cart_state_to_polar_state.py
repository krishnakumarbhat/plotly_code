import numpy as np


def cart_state_to_polar_state(x: np.array, y: np.array, vx: np.array, vy: np.array):
    range = np.hypot(x, y)
    azimuth = np.arctan2(y, x)
    sin_az = np.sin(azimuth)
    cos_az = np.cos(azimuth)
    range_rate = cos_az * vx + sin_az * vy
    cross_radial_vel = -sin_az * vx + cos_az * vy
    return range, azimuth, range_rate, cross_radial_vel

