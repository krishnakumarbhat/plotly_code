from copy import deepcopy

import numpy as np
import pandas as pd
from AptivDataExtractors.utilities.MathFunctions import calc_position_in_bounding_box

from simulations.radar_object.radar_object_simulation_rand_line import simulate_object_from_randomly_from_line
from simulations.radar_object.radar_object_simulation_rand_reduced import simulate_object_randomly_reduced
from simulations.radar_object.radar_object_simulation_ref_point import simulate_object_from_ref_point

if __name__ == '__main__':
    # Common definitions
    n_samples = 2000
    n_pools = 8
    gt_object = pd.Series({
        'id': 0,
        'position_x': 6.0,
        'position_y': 5.0,
        'velocity_otg_x': 4.0,
        'velocity_otg_y': 0.0,
        'bounding_box_dimensions_x': 4.0,
        'bounding_box_dimensions_y': 2.0,
        'bounding_box_orientation': np.deg2rad(0.0),
        'bounding_box_refpoint_long_offset_ratio': 0.5,
        'bounding_box_refpoint_lat_offset_ratio': 0.5,
        'yaw_rate': np.deg2rad(0.0),  # [rad/s]
    })
    c_x, c_y = calc_position_in_bounding_box(gt_object.position_x, gt_object.position_y,
                                             gt_object.bounding_box_dimensions_x,
                                             gt_object.bounding_box_dimensions_y,
                                             gt_object.bounding_box_orientation,
                                             gt_object.bounding_box_refpoint_long_offset_ratio,
                                             gt_object.bounding_box_refpoint_lat_offset_ratio,
                                             np.array([0.5]), np.array([0.5]))
    meas_bias = np.zeros(3)
    gt_object['center_x'] = c_x
    gt_object['center_y'] = c_y
    # Ref points
    ref_points = np.array([[0.0, 0.0],
                           [0.5, 0.0],
                           [1.0, 0.0],
                           [0.0, 0.5],
                           [0.0, 1.0],
                           [0.5, 0.5],
                           [0.01, 0.01],
                           [0.0, 0.01],
                           [0.01, 0.0],
                           [0.05, 0.05],
                           [0.05, 0.0],
                           [0.0, 0.05],
                           [0.1, 0.1]])

    for ref_point in ref_points:
        rp_x = str(ref_point[0]).replace('.', '_')
        rp_y = str(ref_point[1]).replace('.', '_')
        out_path = r'private\sim_ref_point_x' + rp_x + '_y' + rp_y + '_n' + str(n_samples) + '.pickle'
        simulate_object_from_ref_point(gt_object, ref_point, meas_bias,
                                       n_samples,
                                       out_path, True, n_pools)

    # Lines
    lines = np.array([[0.0, 1.0, 0.0, 1.0],
                      [0.0, 1.0, 0.0, 0.0],
                      [0.0, 0.0, 0.0, 1.0],
                      [0.0, 1.0, 0.0, 0.5],
                      [0.0, 0.5, 0.0, 1.0]])

    for line in lines:
        end_x = str(line[1]).replace('.', '_')
        end_y = str(line[3]).replace('.', '_')
        out_path = r'private\sim_line_end_ref_point_x' + end_x + '_y' + end_y + '_n' + str(n_samples) + '.pickle'
        simulate_object_from_randomly_from_line(gt_object, meas_bias,
                                                line[0], line[1], line[2], line[3],
                                                n_samples, out_path, True, n_pools)

    # Random
    n_samples = 5000
    dimensions = np.array([[4.0, 2.0],
                           [2.0, 2.0],
                           [2.0, 1.0],
                           [1.0, 1.0],
                           [0.5, 0.5]])
    for dimension in dimensions:
        gt_object_modified = deepcopy(gt_object)
        gt_object_modified.bounding_box_dimensions_x = dimension[0]
        gt_object_modified.bounding_box_dimensions_y = dimension[1]
        dim_x = str(dimension[0]).replace('.', '_')
        dim_y = str(dimension[1]).replace('.', '_')
        out_path = r'private\sim_rand_reduced_length' + dim_x + '_width' + dim_y + '_n' + str(n_samples) + '.pickle'
        simulate_object_randomly_reduced(gt_object_modified, n_samples, out_path, True, n_pools)


