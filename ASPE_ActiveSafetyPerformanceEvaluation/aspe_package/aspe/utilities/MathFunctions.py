# coding=utf-8
"""
Set of supporting math functions
"""
import math
from typing import Tuple, Union

import numpy as np


def calc_velocity_in_position(in_pos_long: Union[float, np.ndarray], in_pos_lat: Union[float, np.ndarray],
                              in_vel_long: Union[float, np.ndarray], in_vel_lat: Union[float, np.ndarray],
                              yaw_rate: Union[float, np.ndarray],
                              out_pos_long: Union[float, np.ndarray], out_pos_lat: Union[float, np.ndarray]) \
        -> Tuple[Union[float, np.ndarray], Union[float, np.ndarray]]:
    """
    Function calculates velocity at given point (out_pos_long, out_pos_lat) having motion: (in_vel_long, in_vel_lat,
    yaw_rate) at point (in_pos_long, in_pos_lat).
    It uses rigid body theory calculate velocity vectors at different parts of rigid body with given yaw rate
    :param in_pos_long: longitudinal position for input velocity [m]
    :type Union[float, np.ndarray]
    :param in_pos_lat: lateral position for input velocity [m]
    :type Union[float, np.ndarray]
    :param in_vel_long: longitudinal input velocity [m/s]
    :type Union[float, np.ndarray]
    :param in_vel_lat: lateral input velocity [m/s]
    :type Union[float, np.ndarray]
    :param yaw_rate: yaw rate of rigid body [rad/s]
    :type Union[float, np.ndarray]
    :param out_pos_long: longitudinal position for output velocity - point for which velocity should be calculated [m]
    :type Union[float, np.ndarray]
    :param out_pos_lat: lateral position for output velocity - point for which velocity should be calculated  [m]
    :type Union[float, np.ndarray]
    :return: Tuple[Union[float, np.ndarray], Union[float, np.ndarray]], output velocity long and lat [m/s]
    """
    pos_diff_long = out_pos_long - in_pos_long
    pos_diff_lat = out_pos_lat - in_pos_lat

    out_vel_long = in_vel_long - yaw_rate * pos_diff_lat
    out_vel_lat = in_vel_lat + yaw_rate * pos_diff_long
    return out_vel_long, out_vel_lat


def calc_acceleration_in_position(in_pos_long: np.ndarray, in_pos_lat: np.ndarray,
                                  in_acc_long: np.ndarray, in_acc_lat: np.ndarray,
                                  yaw_rate: np.ndarray,
                                  yaw_acceleration: np.ndarray,
                                  out_pos_long: np.ndarray, out_pos_lat: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """
    Function calculates acceleration at given point (out_pos_long, out_pos_lat) having motion: (in_vel_long, in_vel_lat,
    yaw_rate, yaw_acceleration) at point (in_pos_long, in_pos_lat).
    It uses rigid body theory calculate acceleration vectors at different parts of rigid body with given yaw rate and
    yaw acceleration
    :param in_pos_long: longitudinal position for input velocity [m]
    :type np.ndarray
    :param in_pos_lat: lateral position for input velocity [m]
    :type np.ndarray
    :param in_acc_long: longitudinal input acceleration [m/s^2]
    :type np.ndarray
    :param in_acc_lat: lateral input acceleration [m/s^2]
    :type np.ndarray
    :param yaw_rate: yaw rate of rigid body [rad/s]
    :type np.ndarray
    :param yaw_acceleration: yaw acceleration of rigid body [rad/s^2]
    :type np.ndarray
    :param out_pos_long: longitudinal position for output acceleration - point for which
        acceleration should be calculated [m]
    :type np.ndarray
    :param out_pos_lat: lateral position for output acceleration - point for which
        acceleration should be calculated [m]
    :type np.ndarray
    :return: Tuple[np.ndarray, np.ndarray], output acceleration long and lat [m/s]
    """
    pos_diff_long = out_pos_long - in_pos_long
    pos_diff_lat = out_pos_lat - in_pos_lat

    out_acc_long = in_acc_long - yaw_acceleration * pos_diff_lat - (np.square(yaw_rate) * pos_diff_long)
    out_acc_lat = in_acc_lat + yaw_acceleration * pos_diff_long - (np.square(yaw_rate) * pos_diff_lat)
    return out_acc_long, out_acc_lat


def calc_position_in_bounding_box(bbox_pos_lon: np.ndarray, bbox_pos_lat: np.ndarray,
                                  bbox_length: np.ndarray, bbox_width: np.ndarray,
                                  bounding_box_orientation: np.ndarray,
                                  in_ref_point_lon: np.ndarray, in_ref_point_lat: np.ndarray,
                                  out_ref_point_long: np.ndarray, out_ref_point_lat: np.ndarray) \
        -> Tuple[np.ndarray, np.ndarray]:
    """
    Calculate position of given reference point within given bounding box. Bounding box is defined by position of
    reference point (centroid), dimensions - length and width and ratios of reference point location according
    to box boundaries. Ratios are in range 0 - 1 where point (0,0) represents rear, left corner and point (1, 1)
    represents front, right corner.
    :param bbox_pos_lon: array of longitudinal positions of bbox reference points
    :param bbox_pos_lat: array of lateral positions of bbox reference points
    :param bbox_length: array of lengths of bbox longitudinal borders
    :param bbox_width: array of widths of bbox lateral borders
    :param bounding_box_orientation: array pointing angles of bbox
    :param in_ref_point_lon: array of values between 0-1 representing longitudinal position of reference point in bbox,
                             0 - on rear border, 1 on front border
    :param in_ref_point_lat: array of values between 0-1 representing lateral position of reference point in bbox,
                             0 - on left border, 1 - on right border
    :param out_ref_point_long: lateral position of point inside bbox in reference to it dimensions
                               in which position is calculated
    :param out_ref_point_lat: longitudinal position of point inside bbox in reference to it dimensions
                              in which position is calculated
    :return: tuple of longitudinal and lateral position of new calculater reference point
    """
    """
    Take un-rotated bounding box and place it in coordinate system origin. Find place of left - rear corner
    """
    rear_left_point_lat = - in_ref_point_lat * bbox_width
    rear_left_point_long = - in_ref_point_lon * bbox_length

    """
    Find new reference point position for un-rotated bbox placed in coordinate system origin.
    """
    new_ref_point_lat = rear_left_point_lat + bbox_width * out_ref_point_lat
    new_ref_point_lon = rear_left_point_long + bbox_length * out_ref_point_long

    """
    Rotate bbox and move it from origin to right place.
    """
    cos_bounding_box_orientation = np.cos(bounding_box_orientation)
    sin_bounding_box_orientation = np.sin(bounding_box_orientation)

    position_lon = (bbox_pos_lon + new_ref_point_lon * cos_bounding_box_orientation -
                    new_ref_point_lat * sin_bounding_box_orientation)
    position_lat = (bbox_pos_lat + new_ref_point_lon * sin_bounding_box_orientation +
                    new_ref_point_lat * cos_bounding_box_orientation)
    return position_lon, position_lat


def calc_bbox_closest_refernce_point(bbox_center_x: np.ndarray, bbox_center_y: np.ndarray,
                                     bbox_length: np.ndarray, bbox_width: np.ndarray,
                                     bounding_box_orientation: np.ndarray):
    """
    Calculate position of closest reference point within given bounding box. Bounding box is defined by position of
    center position, dimensions - length and width.
    Ratios are in range 0 - 1 where point (0,0) represents rear, left corner and point (1, 1)
    represents front, right corner.
     0 FL - 1 FC - 2 FR
     |             |
     |             |
     7 CL          3 CR
     |             |
     |             |
     6 RL - 5 RC - 4 RR
    :param bbox_center_x: array of center longitudinal positions of bbox reference points
    :param bbox_center_y: array of center lateral positions of bbox reference points
    :param bbox_length: array of lengths of bbox longitudinal borders
    :param bbox_width: array of widths of bbox lateral borders
    :param bounding_box_orientation: array pointing angles of bbox
    :return: closest reference longitudinal and lateral position of new calculated reference point
    """
    rear_left_point_lat = - 0.5 * bbox_width
    rear_left_point_long = - 0.5 * bbox_length

    """
    Find new reference point position for un-rotated bbox placed in coordinate system origin.
    """

    reference_points = np.array(
        [[1.0, 0.0],
         [1.0, 0.5],
         [1.0, 1.0],
         [0.5, 1.0],
         [0.0, 1.0],
         [0.0, 0.5],
         [0.0, 0.0],
         [0.5, 0.0]])

    nr_filed = bbox_center_x.size
    position_lon = np.empty([nr_filed, 8])
    position_lat = np.empty([nr_filed, 8])

    """
    calc_position_in_bounding_box for each posible reference point
    """

    for i, (reference_long_points, reference_lat_points) in enumerate(reference_points):
        position_lon[:, i], position_lat[:, i] = calc_position_in_bounding_box(bbox_center_x, bbox_center_y,
                                                                               bbox_length, bbox_width,
                                                                               bounding_box_orientation,
                                                                               0.5, 0.5,
                                                                               reference_long_points,
                                                                               reference_lat_points)
    """
    determine which reference point is the closest one
    """
    distance_sq = np.hypot(position_lon, position_lat)

    np.expand_dims(np.min(distance_sq, axis=1), axis=1)
    ai = np.expand_dims(np.argmin(distance_sq, axis=1), axis=1)

    closest_ref_posn_long = reference_points[ai, 0].reshape(-1)
    closest_ref_posn_lat = reference_points[ai, 1].reshape(-1)

    return closest_ref_posn_long, closest_ref_posn_lat


def normalize_angle_vector(angle_vector):
    """
    Function to normalize angle vector from [0, 2PI] to [-PI, PI]
    :param angle_vector: ndarray with angle values given in radians
    :return: ndarray of normalized angle values
    >>> angle_vector = np.deg2rad(np.array([0, 1, 45, 90, 135, 179, 181, 270, 359, -340, -170]))
    >>> normalized_vector = normalize_angle_vector(angle_vector)
    >>> print(np.rad2deg(normalized_vector))
    """
    over_pi = angle_vector > np.pi

    angle_vector_new = np.copy(angle_vector)

    under_pi = angle_vector < -np.pi
    angle_vector_new[under_pi] = angle_vector_new[under_pi] % (2 * np.pi)

    angle_vector_new[over_pi] = -(2 * np.pi - angle_vector_new[over_pi])
    return angle_vector_new


def vcs2scs(x_vcs, y_vcs, radar_x_vcs, radar_y_vcs, radar_boresight_angle):
    """
    Function for transforming long and lat position from VSC to SCS
    :param x_vcs:
    :param y_vcs:
    :param radar_x_vcs:
    :param radar_y_vcs:
    :param radar_boresight_angle:
    :return:
    """
    vcs_x_temp = x_vcs - radar_x_vcs
    vcs_y_temp = y_vcs - radar_y_vcs
    x_scs, y_scs = rot_2d_sae_cs(vcs_x_temp, vcs_y_temp, radar_boresight_angle)

    return x_scs, y_scs


def lcs2vcs(x_lcs, y_lcs, lcs_x_origin_in_vcs, lcs_y_origin_in_vcs, lcs_orientation_in_vcs):
    """
    Function for transforming long and lat position from Local Coordinate System to Vehicle Coordinate System
    :param x_lcs:
    :param y_lcs:
    :param lcs_x_origin_in_vcs:
    :param lcs_y_origin_in_vcs:
    :param lcs_orientation_in_vcs:
    :return:
    """
    x_vcs_rot, y_vcs_rot = rot_2d_sae_cs(x_lcs, y_lcs, -lcs_orientation_in_vcs)
    x_vcs = x_vcs_rot + lcs_x_origin_in_vcs
    y_vcs = y_vcs_rot + lcs_y_origin_in_vcs

    return x_vcs, y_vcs


def scs2vcs(x_scs, y_scs, radar_x_vcs, radar_y_vcs, radar_boresight_angle):
    """
    Function for transforming long and lat position from SSC to VCS
    :param x_scs:
    :param y_scs:
    :param radar_x_vcs:
    :param radar_y_vcs:
    :param radar_boresight_angle:
    :return:
    """
    x_vcs, y_vcs = lcs2vcs(x_scs, y_scs, radar_x_vcs, radar_y_vcs, radar_boresight_angle)

    return x_vcs, y_vcs


def rot_2d_sae_cs(x, y, angle):
    """
    Rotate vector in delphi coordinate system. The rotation is done in opposite direction, that's why lat
    is calculated with -sin, and long is calculated with +sin.
    :param x:
    :param y:
    :param angle:
    :return: rotated x, rotated y
    """
    sin_ba = np.sin(angle)
    cos_ba = np.cos(angle)

    x_rot = (cos_ba * x) + (sin_ba * y)
    y_rot = (-sin_ba * x) + (cos_ba * y)

    return x_rot, y_rot


def cart2pol(x, y):
    """
    Convert variables from cartesian to polar coordinate system
    :param x: Logitudinal dimension
    :param y: Lateral dimension
    :return: Range, Angle(radians)
    """
    rho = np.sqrt(x ** 2 + y ** 2)
    phi = np.arctan2(y, x)
    return rho, phi


def pol2cart(rho, phi):
    """
    Convert variables from polar to cartesian coordinate system
    :param rho: Range
    :param phi: Angle(radians)
    :return: Logitudinal dimension, Lateral dimension
    """
    x = rho * np.cos(phi)
    y = rho * np.sin(phi)
    return x, y


def calculate_jacobian_matrix_values_polar_to_cartesian(r, phi):
    """
    Calculates Jacobian matrix values for a transformation from polar to Cartesian coordinate system.
    Accepts series of data (e.g. np.array or pd.Series).
    See https://en.wikipedia.org/wiki/List_of_common_coordinate_transformations#From_polar_coordinates for details.
    :param r:
    :param phi:
    :return: a 4-tuple of (j_11, j_12, j_21, j_22)
    """
    j_11 = np.cos(phi)
    j_21 = np.sin(phi)
    j_12 = -r * j_21  # = -r * np.sin(phi)
    j_22 = r * j_11  # = r * np.cos(phi)
    return j_11, j_12, j_21, j_22


def calculate_rotation_matrix_values(theta):
    """
    Calculates Jacobian matrix values for a rotation.
    Accepts series of data (e.g. np.array or pd.Series).
    See https://en.wikipedia.org/wiki/Rotation_matrix for details.
    :param theta:
    :return: a 4-tuple of (j_11, j_12, j_21, j_22)
    """
    j_11 = j_22 = np.cos(theta)
    j_21 = np.sin(theta)
    j_12 = -j_21  # = -np.sin(theta)
    return j_11, j_12, j_21, j_22

def euler_from_quaternion(w, x, y, z):
    """
    Convert a quaternion into euler angles (roll, pitch, yaw)
    roll is rotation around x in radians (counterclockwise)
    pitch is rotation around y in radians (counterclockwise)
    yaw is rotation around z in radians (counterclockwise)
    """
    t0 = 2.0 * (w * x + y * z)
    t1 = 1.0 - 2.0 * (x * x + y * y)
    roll_x = math.atan2(t0, t1)

    t2 = 2.0 * (w * y - z * x)
    t2 = 1.0 if t2 > 1.0 else t2
    t2 = -1.0 if t2 < -1.0 else t2
    pitch_y = math.asin(t2)

    t3 = 2.0 * (w * z + x * y)
    t4 = 1.0 - 2.0 * (y * y + z * z)
    yaw_z = math.atan2(t3, t4)

    return roll_x, pitch_y, yaw_z  # in radians
