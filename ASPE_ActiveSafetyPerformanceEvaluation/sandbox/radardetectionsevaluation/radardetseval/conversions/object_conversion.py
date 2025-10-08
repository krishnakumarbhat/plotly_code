import numpy as np
from AptivDataExtractors.utilities.MathFunctions import calc_velocity_in_position, rot_2d_sae_cs
from AptivDataExtractors.utilities.UncertaintyPropagation import propagate_uncertainty


def otg_to_rel_vel(velocity_otg_x, velocity_otg_y,
                   cs_origin_velocity_otg_x, cs_origin_velocity_otg_y, cs_yaw_rate,
                   position_x, position_y):
    """
    Calculate relative (rel) velocity in coordinate system (cs) based on over-the-ground (otg) velocity using rigid
    body theory
    :param velocity_otg_x:
    :param velocity_otg_y:
    :param cs_origin_velocity_otg_x:
    :param cs_origin_velocity_otg_y:
    :param cs_yaw_rate:
    :param position_x:
    :param position_y:
    :return:
    """

    cs_yr_influence_x, cs_yr_influence_y = calc_velocity_in_position(0.0, 0.0, 0.0, 0.0, cs_yaw_rate,
                                                                     position_x, position_y)

    velocity_rel_x = velocity_otg_x - cs_origin_velocity_otg_x - cs_yr_influence_x
    velocity_rel_y = velocity_otg_y - cs_origin_velocity_otg_y - cs_yr_influence_y

    return velocity_rel_x, velocity_rel_y


def otg_to_rel_vel_cov(velocity_otg_variance_x, velocity_otg_variance_y, velocity_otg_covariance,
                       cs_velocity_otg_variance_x, cs_velocity_otg_variance_y, cs_velocity_otg_covariance,
                       cs_yaw_rate_variance, position_x, position_y):

    yaw_rate_influence_variance_x, yaw_rate_influence_variance_y, yaw_rate_influence_covariance = \
        yaw_rate_influence_cov_transformation(cs_yaw_rate_variance, position_x, position_y)

    velocity_rel_variance_x = velocity_otg_variance_x + cs_velocity_otg_variance_x + yaw_rate_influence_variance_x
    velocity_rel_variance_y = velocity_otg_variance_y + cs_velocity_otg_variance_y + yaw_rate_influence_variance_y
    velocity_rel_covariance = velocity_otg_covariance + cs_velocity_otg_covariance + yaw_rate_influence_covariance

    return velocity_rel_variance_x, velocity_rel_variance_y, velocity_rel_covariance


def vcs2scs_sensor_velocity(host_velocity_x_vcs, host_velocity_y_vcs, host_yaw_rate,
                            radar_position_x_vcs, radar_position_y_vcs, radar_boresight_angle):
    """
    Calculate sensor over-the-ground (otg) velocity in sensor coordinate system (scs) based on vehicle otg velocity
    defined in vehicle coordinate system (vcs) using rigid body theory
    :param host_velocity_x_vcs:
    :param host_velocity_y_vcs:
    :param host_yaw_rate:
    :param radar_position_x_vcs:
    :param radar_position_y_vcs:
    :param radar_boresight_angle:
    :return:
    """

    vcs_sensor_velocity_otg_x, vcs_sensor_velocity_otg_y = calc_velocity_in_position(0.0, 0.0,
                                                                                     host_velocity_x_vcs,
                                                                                     host_velocity_y_vcs,
                                                                                     host_yaw_rate,
                                                                                     radar_position_x_vcs,
                                                                                     radar_position_y_vcs)

    scs_sensor_velocity_otg_x, scs_sensor_velocity_otg_y = rot_2d_sae_cs(vcs_sensor_velocity_otg_x,
                                                                         vcs_sensor_velocity_otg_y,
                                                                         radar_boresight_angle)
    return scs_sensor_velocity_otg_x, scs_sensor_velocity_otg_y


def vcs2scs_sensor_velocity_cov(vcs_host_velocity_otg_variance_x, vcs_host_velocity_otg_variance_y,
                                vcs_host_velocity_otg_covariance, vcs_host_yaw_rate_variance,
                                vcs_radar_position_x, vcs_radar_position_y, vcs_radar_boresight_angle):
    """

    :param vcs_host_velocity_otg_variance_x:
    :param vcs_host_velocity_otg_variance_y:
    :param vcs_host_velocity_otg_covariance:
    :param vcs_host_yaw_rate_variance:
    :param vcs_radar_position_x:
    :param vcs_radar_position_y:
    :param vcs_radar_boresight_angle:
    :return:
    """

    yaw_rate_influence_variance_x, yaw_rate_influence_variance_y, yaw_rate_influence_covariance = \
        yaw_rate_influence_cov_transformation(vcs_host_yaw_rate_variance, vcs_radar_position_x, vcs_radar_position_y)

    scs_sensor_velocity_otg_variance_x, scs_sensor_velocity_otg_variance_y, scs_sensor_velocity_otg_covariance = \
        rotate_2d_cov(vcs_host_velocity_otg_variance_x + yaw_rate_influence_variance_x,
                      vcs_host_velocity_otg_variance_y + yaw_rate_influence_variance_y,
                      vcs_host_velocity_otg_covariance + yaw_rate_influence_covariance,
                      vcs_radar_boresight_angle)

    return scs_sensor_velocity_otg_variance_x, scs_sensor_velocity_otg_variance_y, scs_sensor_velocity_otg_covariance


def yaw_rate_influence_cov_transformation(yaw_rate_variance, delta_x, delta_y):
    """

    :param yaw_rate_variance:
    :param delta_x:
    :param delta_y:
    :return:
    """
    yaw_rate_influence_variance_x = yaw_rate_variance * (delta_y ** 2)
    yaw_rate_influence_variance_y = yaw_rate_variance * (delta_x ** 2)
    yaw_rate_influence_covariance = yaw_rate_variance * -delta_x * delta_y
    return yaw_rate_influence_variance_x, yaw_rate_influence_variance_y, yaw_rate_influence_covariance


def rotate_2d_cov(x_var, y_var, xy_cov, rotation_angle):
    """

    :param x_var:
    :param y_var:
    :param xy_cov:
    :param rotation_angle:
    :return:
    """

    j_11 = np.full_like(x_var, np.cos(rotation_angle))
    j_12 = np.full_like(x_var, np.sin(rotation_angle))
    j_21 = -j_12
    j_22 = j_11

    x_var_rot, y_var_rot, xy_cov_rot = propagate_uncertainty(x_var, y_var, xy_cov, j_11, j_12, j_21, j_22)
    return x_var_rot, y_var_rot, xy_cov_rot
