from typing import Union

import numpy as np
import pandas as pd


def calc_bbox_corners(center_x: Union[pd.Series, np.ndarray],
                      center_y: Union[pd.Series, np.ndarray],
                      orientation: Union[pd.Series, np.ndarray],
                      length: Union[pd.Series, np.ndarray],
                      width: Union[pd.Series, np.ndarray]):
    """
    Calculate coordinates of bounding box with given center_x, center_y, orientation, length and width. All input
    parameters should be in array form, each array should has equal length N, representing N objects. Output arrays
    containing corners coordinates is in form:
    corners_x: x_obj1_c1, x_obj1_c2, x_obj1_c3, x_obj1_c4, x_obj1_c1, np.nan, x_obj2_c1, x_obj2_c2 ....
    corners_x: y_obj1_c1, y_obj1_c2, y_obj1_c3, y_obj1_c4, y_obj1_c1, np.nan, y_obj2_c1, y_obj2_c2 ....
    Note that first corner is repeated - it is needed for plotting function to close bounding box.
    :param center_x: array of X coordinates of bboxes geometric center
    :param center_y: array of Y coordinates of bboxes geometric center
    :param orientation: array of bboxes orientation angles
    :param length: array of bboxes longitudinal sizes (X axis aligned)
    :param width: array of bboxes lateral sizes (Y axis aligned)
    :return:
    """

    def _change_series_to_numpy_representation(array: Union[pd.Series, np.ndarray]):
        if isinstance(array, pd.Series):
            array = array.to_numpy()
        return array

    center_x = _change_series_to_numpy_representation(center_x)
    center_y = _change_series_to_numpy_representation(center_y)
    orientation = _change_series_to_numpy_representation(orientation)
    length = _change_series_to_numpy_representation(length)
    width = _change_series_to_numpy_representation(width)

    rot_matrix = np.array([
        [np.cos(orientation), np.sin(orientation)],
        [-np.sin(orientation), np.cos(orientation)]]).transpose((2, 0, 1))

    position_matrix = np.array([
        [-length / 2, -width / 2],
        [-length / 2, width / 2],
        [length / 2, width / 2],
        [length / 2, -width / 2],
    ]).transpose((2, 0, 1))

    position_rotated = position_matrix @ rot_matrix
    corners_x = position_rotated[:, :, 0] + np.tile(center_x, (4, 1)).T
    corners_y = position_rotated[:, :, 1] + np.tile(center_y, (4, 1)).T

    corners_len = corners_x.shape[0]
    corners_x = np.hstack([corners_x, corners_x[:, 0].reshape(-1, 1), np.full((corners_len, 1), np.nan)]).reshape(-1)
    corners_y = np.hstack([corners_y, corners_y[:, 0].reshape(-1, 1), np.full((corners_len, 1), np.nan)]).reshape(-1)

    return corners_x, corners_y


def calc_bbox_corners_for_front_center_position(position_x: Union[pd.Series, np.ndarray],
                                                position_y: Union[pd.Series, np.ndarray],
                                                orientation: Union[pd.Series, np.ndarray],
                                                length: Union[pd.Series, np.ndarray],
                                                width: Union[pd.Series, np.ndarray]):
    """
    Calculate coordinates of bounding box with given
    front_end_center_x, front_end_center_y, orientation, length and width. All input
    parameters should be in array form, each array should has equal length N, representing N objects. Output arrays
    containing corners coordinates is in form:
    corners_x: x_obj1_c1, x_obj1_c2, x_obj1_c3, x_obj1_c4, x_obj1_c1, np.nan, x_obj2_c1, x_obj2_c2 ....
    corners_x: y_obj1_c1, y_obj1_c2, y_obj1_c3, y_obj1_c4, y_obj1_c1, np.nan, y_obj2_c1, y_obj2_c2 ....
    Note that first corner is repeated - it is needed for plotting function to close bounding box.
    :param position_x: array of X coordinates of bboxes front end center
    :param position_y: array of Y coordinates of bboxes front end center
    :param orientation: array of bboxes orientation angles
    :param length: array of bboxes longitudinal sizes (X axis aligned)
    :param width: array of bboxes lateral sizes (Y axis aligned)
    :return:
    """

    def _change_series_to_numpy_representation(array: Union[pd.Series, np.ndarray]):
        if isinstance(array, pd.Series):
            array = array.to_numpy()
        return array

    position_x = _change_series_to_numpy_representation(position_x)
    position_y = _change_series_to_numpy_representation(position_y)
    orientation = _change_series_to_numpy_representation(orientation)
    length = _change_series_to_numpy_representation(length)
    width = _change_series_to_numpy_representation(width)

    rot_matrix = np.array([
        [np.cos(orientation), np.sin(orientation)],
        [-np.sin(orientation), np.cos(orientation)]]).transpose((2, 0, 1))

    position_matrix = np.array([
        [-length, -width / 2],
        [-length, width / 2],
        [np.zeros(length.size), width / 2],
        [np.zeros(length.size), -width / 2],
    ]).transpose((2, 0, 1))

    position_rotated = position_matrix @ rot_matrix
    corners_x = position_rotated[:, :, 0] + np.tile(position_x, (4, 1)).T
    corners_y = position_rotated[:, :, 1] + np.tile(position_y, (4, 1)).T

    corners_len = corners_x.shape[0]
    corners_x = np.hstack([corners_x, corners_x[:, 0].reshape(-1, 1), np.full((corners_len, 1), np.nan)]).reshape(-1)
    corners_y = np.hstack([corners_y, corners_y[:, 0].reshape(-1, 1), np.full((corners_len, 1), np.nan)]).reshape(-1)

    return corners_x, corners_y


def flatten_single_scan_corners_data(corners):
    x_corners = \
        corners.loc[:,
        ["corner_x_RL", "corner_x_RR", "corner_x_FR", "corner_x_FL", "corner_x_RL_2", "x_nans"]].to_numpy()
    y_corners = \
        corners.loc[:,
        ["corner_y_RL", "corner_y_RR", "corner_y_FR", "corner_y_FL", "corner_y_RL_2", "y_nans"]].to_numpy()
    df_indexes = corners.index.to_numpy()
    df_indexes = np.tile(df_indexes, (5, 1)).T
    df_indexes = np.hstack([df_indexes, np.full((df_indexes.shape[0], 1), -1)])

    x_corners_flat = x_corners.reshape(-1)
    y_corners_flat = y_corners.reshape(-1)
    df_indexes_flat = df_indexes.reshape(-1)
    return x_corners_flat, y_corners_flat, df_indexes_flat


def flatten_single_scans_corneres_data_diagonal_only(corners):
    x_corners = corners.loc[:, ["corner_x_RL", "corner_x_FR", "x_nans"]].to_numpy()
    y_corners = corners.loc[:, ["corner_y_RL", "corner_y_FR", "y_nans"]].to_numpy()
    df_indexes = corners.index.to_numpy()
    df_indexes = np.tile(df_indexes, (3, 1)).T
    df_indexes = np.hstack([df_indexes, np.full((df_indexes.shape[0], 1), -1)])

    x_corners_flat = x_corners.reshape(-1)
    y_corners_flat = y_corners.reshape(-1)
    df_indexes_flat = df_indexes.reshape(-1)
    return x_corners_flat, y_corners_flat, df_indexes_flat
