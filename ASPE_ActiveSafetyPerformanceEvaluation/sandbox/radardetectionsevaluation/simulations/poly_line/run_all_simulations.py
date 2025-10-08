from simulations.poly_line.arc_rand_simulation import simulate_arc
import numpy as np

if __name__ == '__main__':
    n_samples = 200000

    gt_x = 4.0
    gt_y = 4.0
    gt_vx = 4.0
    gt_vy = 0.0
    speed = np.hypot(gt_vx, gt_vy)
    segment_length = 2.0
    line_orientation = np.deg2rad(90.0)
    n_pools = None

    angles = [5, 15, 30, 45, 60, 75, 85, 90, 95, 105, 120, 135, 150, 165, 175, 180]

    for alpha in angles:
        out_path = r'private\sim_arc_' + str(alpha) + '_degree_vel_' + str(gt_vx) + '_n' + str(n_samples) + '.pickle'
        simulate_arc([gt_x, gt_y, gt_vx, gt_vy], segment_length,
                     line_orientation, np.deg2rad(-alpha), n_samples,
                     out_path, True, n_pools)
