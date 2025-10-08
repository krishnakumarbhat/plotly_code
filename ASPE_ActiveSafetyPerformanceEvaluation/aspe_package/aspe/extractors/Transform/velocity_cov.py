import numpy as np


def otg_to_rel_vel_cov(velocity_otg_variance_x: np.array,
                       velocity_otg_variance_y: np.array,
                       velocity_otg_covariance: np.array,
                       cs_velocity_otg_variance_x: np.array,
                       cs_velocity_otg_variance_y: np.array,
                       cs_velocity_otg_covariance: np.array,
                       cs_yaw_rate_variance: np.array,
                       position_x: np.array,
                       position_y: np.array) \
        -> (np.array, np.array, np.array):
    """
    Transformation of velocity covariance from over-the-ground frame to relative frame
    :param velocity_otg_variance_x: velocity Over-The-Ground variance x of point of interest in given coordinate system
    :type velocity_otg_variance_x: np.array
    :param velocity_otg_variance_y: velocity Over-The-Ground variance y of point of interest in given coordinate system
    :type velocity_otg_variance_y: np.array
    :param velocity_otg_covariance: velocity Over-The-Ground covariance xy of point of interest in given coordinate
           system
    :type velocity_otg_covariance: np.array
    :param cs_velocity_otg_variance_x: velocity Over-The-Ground variance x of coordinate system origin
    :type cs_velocity_otg_variance_x: np.array
    :param cs_velocity_otg_variance_y: velocity Over-The-Ground variance y of coordinate system origin
    :type cs_velocity_otg_variance_y: np.array
    :param cs_velocity_otg_covariance: velocity Over-The-Ground covariance xy of coordinate system origin
    :type cs_velocity_otg_covariance: np.array
    :param cs_yaw_rate_variance: yaw rate variance of coordinate system frame
    :type cs_yaw_rate_variance: np.array
    :param position_x: position x of point of interest in given coordinate system
    :type position_x: np.array
    :param position_y: position y of point of interest in given coordinate system
    :type position_y: np.array
    :return: (np.array, np.array, np.array):
             -velocity_rel_variance_x
             -velocity_rel_variance_y
             -velocity_rel_covariance

    """

    yaw_rate_influence_variance_x, yaw_rate_influence_variance_y, yaw_rate_influence_covariance = \
        yaw_rate_influence_cov_transformation(cs_yaw_rate_variance, position_x, position_y)

    velocity_rel_variance_x = velocity_otg_variance_x + cs_velocity_otg_variance_x + yaw_rate_influence_variance_x
    velocity_rel_variance_y = velocity_otg_variance_y + cs_velocity_otg_variance_y + yaw_rate_influence_variance_y
    velocity_rel_covariance = velocity_otg_covariance + cs_velocity_otg_covariance + yaw_rate_influence_covariance

    return velocity_rel_variance_x, velocity_rel_variance_y, velocity_rel_covariance


def yaw_rate_influence_cov_transformation(yaw_rate_variance: np.array, delta_x: np.array, delta_y: np.array) \
        -> (np.array, np.array, np.array):
    """

    :param yaw_rate_variance: yaw rate variance
    :type yaw_rate_variance: np.array
    :param delta_x: position difference in x direction
    :type delta_x: np.array
    :param delta_y: position difference in y direction
    :type delta_y: np.array
    :return: (np.array, np.array, np.array):
             -yaw_rate_influence_variance_x
             -yaw_rate_influence_variance_y
             -yaw_rate_influence_covariance
    """
    yaw_rate_influence_variance_x = yaw_rate_variance * (delta_y ** 2)
    yaw_rate_influence_variance_y = yaw_rate_variance * (delta_x ** 2)
    yaw_rate_influence_covariance = yaw_rate_variance * -delta_x * delta_y
    return yaw_rate_influence_variance_x, yaw_rate_influence_variance_y, yaw_rate_influence_covariance
