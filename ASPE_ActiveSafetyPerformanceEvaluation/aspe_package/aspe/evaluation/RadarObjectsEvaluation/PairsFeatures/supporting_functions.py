'''
It, although I hate,Full of dummy mistakes, my life is
                                    ____
                         _.' :  `._
                     .-.'`.  ;   .'`.-.
            __      / : ___\ ;  /___ ; \      __
          ,'_ ""--.:__;".-.";: :".-.":__;.--"" _`,
          :' `.t""--.. '<@.`;_  ',@>` ..--""j.' `;
               `:-.._J '-.-'L__ `-- ' L_..-;'
                 "-.__ ;  .-"  "-.  : __.-"
                     L ' /.------.\ ' J
                      "-.   "--"   .-"
                     __.l"-:_JL_;-";.__
                  .-j/'.;  ;""""  / .'\ "-.
                .' /:`. "-.:     .-" .';  `.
             .-"  / ;  "-. "-..-" .-"  :    "-.
          .+"-.  : :      "-.__.-"      ;-._   \
          ; \  `.; ;       YODA        : : "+. ;
          :  ;   ; ;        by          : ;  : \:
         : `."-; ;  ;     Szymon       :  ;   ,/;
          ;    -: ;  :                ;  : .-"'  :
          :\     \  : ;             : \.-"      :
           ;`.    \  ; :            ;.'_..--  / ;
           :  "-.  "-:  ;          :/."      .'  :
             \       .-`.\        /t-""  ":-+.   :
              `.  .-"    `l    __/ /`. :  ; ; \  ;
                \   .-" .-"-.-"  .' .'j \  /   ;/
                 \ / .-"   /.     .'.' ;_:'    ;
                  :-""-.`./-.'     /    `.___.'
                        \ `t  ._  /  bug :F_P:
                         "-.t-._:'
'''
from typing import Callable, Union

import numpy as np


def adjust_pos_by_reference_point_2_reference_point(baseline_object_data, source_object_data):
    """
    Adjust source position to same reference point as in baseline object
    :param baseline_object_data: baseline object with reference point described
    :type baseline_object_data: DataFrame
    :param source_object_data: (it will be modified) objects to adjust position
    :type source_object_data: DataFrame
    :return: None
    """
    from aspe.utilities.MathFunctions import calc_position_in_bounding_box
    origin_ref_point_long = source_object_data.bounding_box_refpoint_long_offset_ratio.values
    origin_ref_point_lat = source_object_data.bounding_box_refpoint_lat_offset_ratio.values
    origin_pos_long = source_object_data.position_x.values
    origin_pos_lat = source_object_data.position_y.values
    new_ref_point_long = baseline_object_data.bounding_box_refpoint_long_offset_ratio.values
    new_ref_point_lat = baseline_object_data.bounding_box_refpoint_lat_offset_ratio.values
    dim_long = source_object_data.bounding_box_dimensions_x.values
    dim_lat = source_object_data.bounding_box_dimensions_y.values
    orientation = source_object_data.bounding_box_orientation.values

    new_long, new_lat = calc_position_in_bounding_box(origin_pos_long, origin_pos_lat, dim_long, dim_lat,
                                                      orientation, origin_ref_point_long, origin_ref_point_lat,
                                                      new_ref_point_long, new_ref_point_lat)

    source_object_data.position_x = new_long
    source_object_data.position_y = new_lat
    source_object_data.bounding_box_refpoint_long_offset_ratio = new_ref_point_long
    source_object_data.bounding_box_refpoint_lat_offset_ratio = new_ref_point_lat


def calculate_nees_values_2d(dev_x, dev_y, var_x, var_y, cov_x_y):
    """
    Calculates normalized estimated error square (NEES) value.

    Let D be the deviation vector [dev_x, dev_y] = [est_x - ref_x, est_y - ref_y],
    and Sigma be the covariance matrix [ [var_x, cov_x_y], [cov_x_y, var_y] ].

    NEES value can be calculated using the formula: D^T @ Sigma^-1 @ D.
    Note that Mahalanobis distance = sqrt(NEES value).
    See https://en.wikipedia.org/wiki/Mahalanobis_distance#Definition_and_properties for details.

    In case of nees_value_x and nees_value_y, deviations are one-dimensional,
    so the formula D^T @ Sigma^-1 @ D reduces to dev_x * var_x^-1 * dev_x = dev_x^2 / var_x.
    :param dev_x: deviations of X
    :param dev_y: deviations of Y
    :param var_x: variances of X
    :param var_y: variances of Y
    :param cov_x_y: covariances of X and Y
    :return: triple (nees_value_x, nees_value_y, nees_value_xy) where:
        nees_value_x is NEES value for X only,
        nees_value_y is NEES value for Y only,
        nees_value_xy is NEES value for vector [X, Y].
    """
    nees_value_x = dev_x ** 2 / var_x
    nees_value_y = dev_y ** 2 / var_y

    cov_matrices_3d = np.array([
        [var_x, cov_x_y],
        [cov_x_y, var_y],
    ]).transpose((2, 0, 1))

    inv_cov_matrices_3d = np.linalg.pinv(cov_matrices_3d)

    dev_xy_3d = np.array([
        [dev_x],
        [dev_y],
    ]).transpose((2, 0, 1))

    dev_xy_T_3d = dev_xy_3d.swapaxes(1, 2)

    nees_value_xy_3d = dev_xy_T_3d @ inv_cov_matrices_3d @ dev_xy_3d
    nees_value_xy = nees_value_xy_3d.ravel()

    return nees_value_x, nees_value_y, nees_value_xy


def moving_window(signal_in: np.ndarray, win_size: int, win_fun: Callable[[np.ndarray], float], step: int=1):
    idx_mid = 0
    windowed_out = []
    while idx_mid < len(signal_in):
        idx_beg = int(saturate_downwards(value=(idx_mid - win_size / 2), min_value=0))
        idx_end = int(saturate_upwards(value=(idx_mid + win_size / 2), max_value=len(signal_in)))
        sub_arr = signal_in[idx_beg: idx_end]
        windowed_out.append(win_fun(sub_arr))
        idx_mid += step
    return np.array(windowed_out)


def saturate_downwards(value: Union[int, float], min_value: Union[int, float]):
    output = value
    if value < min_value:
        output = min_value
    return output


def saturate_upwards(value: Union[int, float], max_value: Union[int, float]):
    output = value
    if max_value < value:
        output = max_value
    return output
