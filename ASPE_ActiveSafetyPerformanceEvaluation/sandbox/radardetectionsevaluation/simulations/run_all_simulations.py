import numpy as np
from simulations.single_det.single_det_simulation import simulate_single_point
from simulations.single_segment.single_lane_same_point_end_and_middle import simulate_mid_and_end_point
from simulations.single_segment.single_segment_simulation import simulate_single_line

n_simulations = 10000

gt_x = 4.0
gt_y = -4.0
gt_vx = 4.0
gt_vy = 0.0
single_line_length = 16.0
mid_end_line_length = 8.0
line_orientation = np.deg2rad(90.0)

single_line_prefix = r'single_segment\private\sim_single_line_out_'
mid_and_end_prefix = r'single_segment\private\sim_line_middle_and_end_out_'
"""
simulate_single_point([gt_x, gt_y, gt_vx, gt_vy],
                      n_simulations,
                      r'single_det\private\sim_single_point_x4_00_y4_00_vx4_00_vy_00.pickle')
suffix = 'vel_4'
simulate_single_line([gt_x, gt_y, gt_vx, gt_vy],
                     [gt_x, gt_y+single_line_length, gt_vx, gt_vy],
                     n_simulations, single_line_prefix + suffix + '.pickle')

simulate_mid_and_end_point(gt_x, gt_y, gt_vx, gt_vy, mid_end_line_length, line_orientation,
                           n_simulations, mid_and_end_prefix + suffix + '.pickle')

# Velocity vector perpendicular to azimuth
suffix = 'vel_perp_to_azimuth'
simulate_single_line([gt_x, gt_y, gt_vx, gt_vx],
                     [gt_x, gt_y+single_line_length, gt_vx, gt_vx],
                     n_simulations, single_line_prefix + suffix + '.pickle')

simulate_mid_and_end_point(gt_x, gt_y, gt_vx, gt_vx, mid_end_line_length, line_orientation,
                           n_simulations, mid_and_end_prefix + suffix + '.pickle')

# Velocity vector inline with azimuth
suffix = 'vel_inline_with_azimuth'
simulate_single_line([gt_x, gt_y, gt_vx, -gt_vx],
                     [gt_x, gt_y+single_line_length, gt_vx, -gt_vx],
                     n_simulations, single_line_prefix + suffix + '.pickle')

simulate_mid_and_end_point(gt_x, gt_y, gt_vx, -gt_vx, mid_end_line_length, line_orientation,
                           n_simulations, mid_and_end_prefix + suffix + '.pickle')

# Velocity zero
suffix = 'vel_0'
simulate_single_point([gt_x, gt_y, 0.0, 0.0],
                      n_simulations,
                      r'single_det\private\sim_single_point_x4_00_y4_00_vx0_00_vy_00.pickle')

simulate_single_line([gt_x, gt_y, 0.0, 0.0],
                     [gt_x, gt_y+single_line_length, 0.0, 0.0],
                     n_simulations, single_line_prefix + suffix + '.pickle')

simulate_mid_and_end_point(gt_x, gt_y, 0.0, 0.0, mid_end_line_length, line_orientation,
                           n_simulations, mid_and_end_prefix + suffix + '.pickle')


# Lane orientation in lane with azimuth
suffix = 'segment_inline_with_azimuth'
simulate_single_line([gt_x, gt_y, gt_vx, gt_vy],
                     [gt_x+single_line_length, gt_y-single_line_length, gt_vx, gt_vy],
                     n_simulations, single_line_prefix + suffix + '.pickle')

simulate_mid_and_end_point(gt_x, gt_y, gt_vx, gt_vy, mid_end_line_length, np.deg2rad(-45.0),
                           n_simulations, mid_and_end_prefix + suffix + '.pickle')

# Lane orientation perpendicular to azimuth
suffix = 'segment_perp_to_azimuth'
simulate_single_line([gt_x, gt_y, gt_vx, gt_vy],
                     [gt_x+single_line_length, gt_y+single_line_length, gt_vx, gt_vy],
                     n_simulations, single_line_prefix + suffix + '.pickle')

simulate_mid_and_end_point(gt_x, gt_y, gt_vx, gt_vy, mid_end_line_length, np.deg2rad(45.0),
                           n_simulations, mid_and_end_prefix + suffix + '.pickle')
"""

# Big velocity
suffix = 'vel_40'
vel_scale = 10.0
simulate_single_line([gt_x, gt_y, gt_vx*vel_scale, gt_vy*vel_scale],
                     [gt_x, gt_y+single_line_length, gt_vx*vel_scale, gt_vy*vel_scale],
                     n_simulations, single_line_prefix + suffix + '.pickle')

simulate_mid_and_end_point(gt_x, gt_y, gt_vx*vel_scale, gt_vy*vel_scale, mid_end_line_length, line_orientation,
                           n_simulations, mid_and_end_prefix + suffix + '.pickle')
