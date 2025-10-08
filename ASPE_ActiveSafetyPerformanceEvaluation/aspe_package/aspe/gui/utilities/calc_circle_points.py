from typing import Union

import numpy as np
import pandas as pd


def calc_circle_points(center_x:       Union[pd.Series, np.ndarray],
                       center_y:       Union[pd.Series, np.ndarray],
                       radius:         Union[pd.Series, np.ndarray],
                       max_x_value:    int,
                       min_x_value:    int,
                       line_points:    int,
                       circle_radius_for_straight_line: int,
                       max_theta=2*np.pi):
    """
    Calculate coordinates of circle with given center_x, center_y, radius. All input
    parameters should be in array form, each array should has equal length N, representing N objects. Output arrays
    containing circuit coordinates is in form:
    circuit_x: x_obj1_c1, x_obj1_c2, x_obj1_c3, ..., x_obj1_cn, x_obj1_c1, np.nan, x_obj2_c1, x_obj2_c2 ....
    circuit_y: y_obj1_c1, y_obj1_c2, y_obj1_c3, ..., y_obj1_cn, y_obj1_c1, np.nan, y_obj2_c1, y_obj2_c2 ....
    Note that first point of circuit is repeated - it is needed for plotting function to close circle.
    :param center_x: array of X coordinates of circle geometric center
    :param center_y: array of Y coordinates of circle geometric center
    :param radius: array of circles radius
    :param max_x_value: points with X coordinate above that value will not be calculated
    :param min_x_value: points with X coordinate under that value will not be calculated
    :param line_points: number of line points
    :param circle_radius_for_straight_line: maximum circle radius
    :param max_theta: takes only two values: default 2*np.pi (drawing circles) or np.pi(drawing host predicted path)
    :return:
    """
    def _change_series_to_numpy_representation(array: Union[pd.Series, np.ndarray]):
        if isinstance(array, pd.Series):
            array = array.to_numpy()
        return array
    center_x = _change_series_to_numpy_representation(center_x).reshape(-1, 1)
    center_y = _change_series_to_numpy_representation(center_y).reshape(-1, 1)
    radius = _change_series_to_numpy_representation(radius).reshape(-1, 1)

    theta = np.linspace(0, max_theta, line_points)
    sin_theta = np.sin(theta)
    cos_theta = np.cos(theta)

    circuit_x = radius * cos_theta + center_x
    circuit_y = radius * sin_theta + center_y

    #assignment of commenting a circle that is not in range and turning it into straight line
    radius_too_large = abs(radius.reshape(-1)) > circle_radius_for_straight_line
    circuit_x[radius_too_large, :] = np.hstack([np.array([min_x_value, max_x_value]), np.full(line_points-2, np.nan)])
    circuit_y[radius_too_large, :] = np.hstack([np.array([0, 0]), np.full(line_points-2, np.nan)])

    #limit the display of the x axis
    circuit_x[circuit_x > max_x_value] = np.nan
    circuit_x[circuit_x < min_x_value] = np.nan

    circuit_len = circuit_x.shape[0]
    if max_theta == 2*np.pi:
        first_col_to_repeat_x = circuit_x[:, 0].copy()
        first_col_to_repeat_y = circuit_y[:, 0].copy()
    else:
        first_col_to_repeat_x = np.full([len(circuit_x[:, 0]),1], np.nan)
        first_col_to_repeat_y = np.full([len(circuit_y[:, 0]),1], np.nan)
    first_col_to_repeat_x[radius_too_large] = np.nan
    first_col_to_repeat_y[radius_too_large] = np.nan

    nan_column = np.full((circuit_len, 1), np.nan)

    circuit_x = np.hstack([circuit_x, first_col_to_repeat_x.reshape(-1, 1), nan_column]).reshape(-1)
    circuit_y = np.hstack([circuit_y, first_col_to_repeat_y.reshape(-1, 1), nan_column]).reshape(-1)

    return circuit_x, circuit_y
