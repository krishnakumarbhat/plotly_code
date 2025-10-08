from copy import deepcopy

import numpy as np
import pandas as pd
from AptivDataExtractors.utilities.MathFunctions import calc_position_in_bounding_box

from simulations.radar_object.radar_object_simulation_rand import simulate_object_randomly
from simulations.radar_object.radar_object_simulation_rand_line import simulate_object_from_randomly_from_line
from simulations.radar_object.radar_object_simulation_rand_reduced import simulate_object_randomly_reduced
from simulations.radar_object.radar_object_simulation_ref_point import simulate_object_from_ref_point

if __name__ == '__main__':
    # Common definitions
    n_samples = 100000
    n_pools = 6
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
    azimuth_biases = np.deg2rad(np.linspace(-0.5, 0.5, 21))
    meas_bias = np.zeros(3)
    gt_object['center_x'] = c_x
    gt_object['center_y'] = c_y

    for azimuth_bias in azimuth_biases:
        meas_bias[1] = azimuth_bias
        bias_deg = str(round(np.rad2deg(azimuth_bias), 2)).replace('.', '_')
        out_path = r'private\sim_rand_azimuth_bias_' + bias_deg + '_n' + str(n_samples) + '.pickle'
        simulate_object_randomly(gt_object, meas_bias, n_samples, out_path, True, n_pools)


