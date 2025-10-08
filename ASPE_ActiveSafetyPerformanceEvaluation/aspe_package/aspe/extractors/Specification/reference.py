import numpy as np


class RT3002Spec:
    """
    Specification of RT range 3002
    """
    version = 1
    description = 'Based on RT3002 specification'
    pos_std = 0.1  # [m]
    vel_std = 0.05/3.6  # [m/s]
    yaw_angle_std = np.deg2rad(0.15)  # [rad]

    @staticmethod
    def angle_rate_std(angle_rate):
        bias = np.deg2rad(0.01)  # [rad/s]
        scale_factor = 0.001
        std = abs(angle_rate) * scale_factor + bias
        return std  # [rad/s]
