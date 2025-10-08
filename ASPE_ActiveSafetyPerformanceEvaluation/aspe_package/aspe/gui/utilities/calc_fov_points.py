from typing import Union

import numpy as np
import pandas as pd


def calc_fov_points(radar_x_pos:    Union[pd.Series, np.ndarray],
                    radar_y_pos:    Union[pd.Series, np.ndarray],
                    ref_angle:      Union[pd.Series, np.ndarray],
                    range:          Union[pd.Series, np.ndarray],
                    min_theta:      Union[pd.Series, np.ndarray],
                    max_theta:      Union[pd.Series, np.ndarray],
                    line_points=50):
    """
    Calculate coordinates of radar field of view. All input
    parameters should be in array form, each array should has equal length N, representing N objects. Output arrays
    containing circuit coordinates is in form:
    circuit_x: x_obj1_c1, x_obj1_c2, x_obj1_c3, ..., x_obj1_cn, x_obj1_c1, np.nan, x_obj2_c1, x_obj2_c2 ....
    circuit_y: y_obj1_c1, y_obj1_c2, y_obj1_c3, ..., y_obj1_cn, y_obj1_c1, np.nan, y_obj2_c1, y_obj2_c2 ....
    Note that first point of circuit is repeated - it is needed for plotting function to close circle.
    :param radar_x_pos: array of X coordinates of radar geometric center
    :param radar_y_pos: array of Y coordinates of radar geometric center
    :param ref_angle: boresight angle
    :param min_theta: minimum (initial) angle of the curve
    :param max_theta: maximum (ending) angle of the curve
    :param line_points: number of line points
    :return:
    """
    def _change_series_to_numpy_representation(array: Union[pd.Series, np.ndarray]):
        if isinstance(array, pd.Series):
            array = array.to_numpy()
        return array
    radar_x_pos = _change_series_to_numpy_representation(radar_x_pos).reshape(1, -1)
    radar_y_pos = _change_series_to_numpy_representation(radar_y_pos).reshape(1, -1)
    ref_angle = _change_series_to_numpy_representation(ref_angle).reshape(-1, 1)
    range = _change_series_to_numpy_representation(range).reshape(-1, 1)
    min_theta = _change_series_to_numpy_representation(min_theta)
    max_theta = _change_series_to_numpy_representation(max_theta)

    theta = np.linspace(min_theta, max_theta, line_points)
    theta = ref_angle.reshape(1, -1) + theta
    sin_theta = np.sin(theta)
    cos_theta = np.cos(theta)

    radar_x_pos = np.tile(radar_x_pos, (line_points, 1))
    radar_y_pos = np.tile(radar_y_pos, (line_points, 1))

    fov_x = range * cos_theta.T + radar_x_pos.T
    fov_y = range * sin_theta.T + radar_y_pos.T

    fov_len = fov_x.shape[0]
    col_to_repeat_x = radar_x_pos[1, :].reshape(-1,1)
    col_to_repeat_y = radar_y_pos[1, :].reshape(-1,1)
    nan_column = np.full((fov_len, 1), np.nan)

    fov_x = np.hstack([col_to_repeat_x, fov_x, col_to_repeat_x, nan_column]).reshape(-1)
    fov_y = np.hstack([col_to_repeat_y, fov_y, col_to_repeat_y, nan_column]).reshape(-1)

    return fov_x, fov_y