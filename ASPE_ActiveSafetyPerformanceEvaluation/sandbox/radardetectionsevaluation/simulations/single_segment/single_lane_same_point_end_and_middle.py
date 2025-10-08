import pickle
import numpy as np
import pandas as pd
from tqdm import tqdm
from radardetseval.Iface.PolyLineDS import PolyLineDS
from radardetseval.association.nearest_point_on_line import find_nearest_point_on_polyline_minimize
from radardetseval.configs.default_radar import DefaultRadar
from radardetseval.configs.defaut_reference import DefaultReference
from radardetseval.simulators.DetFromPolyLineSim import DetFromPolyLineSim
from radardetseval.simulators.PolyLineSim import PolyLineSim


def simulate_mid_and_end_point(point_of_interest_x=4.0, point_of_interest_y=4.0, gt_vel_x=4.0, gt_vel_y=0.0,
                               line_length=8.0, line_orientation=np.deg2rad(90.0), n_simulations=10000,
                               data_path=r'private\sim_line_middle_and_end_out.pickle'):
    print(f'Simulate lane for middle point and end point use case with:')
    print(f'point_of_interest_x: {point_of_interest_x:.1f}, point_of_interest_y: {point_of_interest_y:.1f}, '
          f'gt_vel_x: {gt_vel_x:.1f}, gt_vel_y: {gt_vel_y:.1f},')
    print(f'line_length: {line_length:.1f}, line_orientation: {line_orientation:.1f},')
    print(f'n_simulations: {n_simulations}')
    print(f'data_path: {data_path}')

    ref_pos_std = DefaultReference.pos_std
    ref_vel_std = DefaultReference.vel_std

    meas_cov = np.power(np.diag([DefaultRadar.range_std, DefaultRadar.azimuth_std, DefaultRadar.range_rate_std]), 2.0)
    ref_cov = np.power(np.diag([ref_pos_std, ref_pos_std, ref_vel_std, ref_vel_std]), 2)

    end_point_x = point_of_interest_x + line_length * np.cos(line_orientation)
    end_point_y = point_of_interest_y + line_length * np.sin(line_orientation)

    rel_move_x = (end_point_x - point_of_interest_x) * 0.5
    rel_move_y = (end_point_y - point_of_interest_y) * 0.5

    gt_polyline_end = PolyLineDS()
    gt_polyline_end.add_point(point_of_interest_x, point_of_interest_y, gt_vel_x, gt_vel_y)
    gt_polyline_end.add_point(end_point_x, end_point_y, gt_vel_x, gt_vel_y)

    gt_polyline_middle = PolyLineDS()
    gt_polyline_middle.add_point(point_of_interest_x - rel_move_x, point_of_interest_y - rel_move_y, gt_vel_x, gt_vel_y)
    gt_polyline_middle.add_point(point_of_interest_x + rel_move_x, point_of_interest_y + rel_move_y, gt_vel_x, gt_vel_y)

    polyline_simulator_end = PolyLineSim(gt_polyline_end)
    det_simulator_end = DetFromPolyLineSim(gt_polyline_end)

    polyline_simulator_middle = PolyLineSim(gt_polyline_middle)
    det_simulator_middle = DetFromPolyLineSim(gt_polyline_middle)

    sim_out_end = list()
    sim_out_middle = list()

    for _ in tqdm(range(n_simulations)):
        polyline_end = polyline_simulator_end.sim_with_the_same_deviation(ref_cov)
        detection_end = det_simulator_end.sim_single_point_from_vertex(meas_cov, vertex_id=0.0)
        sim_out_end.append(find_nearest_point_on_polyline_minimize(polyline_end, detection_end))

        polyline_middle = polyline_simulator_middle.sim_with_the_same_deviation(ref_cov)
        detection_middle = det_simulator_middle.sim_single_point_from_vertex(meas_cov, vertex_id=0.5)
        sim_out_middle.append(find_nearest_point_on_polyline_minimize(polyline_middle, detection_middle))

    data = {'gt_polyline_end': gt_polyline_end,
            'gt_polyline_middle': gt_polyline_middle,
            'sim_out_end_df': pd.DataFrame(sim_out_end),
            'sim_out_middle_df': pd.DataFrame(sim_out_middle),
            'n_simulations': n_simulations}

    with open(data_path, 'wb') as handle:
        pickle.dump(data, handle, protocol=pickle.HIGHEST_PROTOCOL)


if __name__ == '__main__':
    simulate_mid_and_end_point(4.0, -4.0, 4.0, 0.0, 8.0, np.deg2rad(90.0), 100,
                               r'private\sim_line_middle_and_end_out_ref_vel_4.pickle')
