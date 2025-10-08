import pickle
import numpy as np
import pandas as pd
from tqdm import tqdm
from radardetseval.Iface.PolyLineDS import PolyLineDS
from radardetseval.association.nearest_point_on_line import find_nearest_point_on_segment_minimize
from radardetseval.configs.default_radar import DefaultRadar
from radardetseval.configs.defaut_reference import DefaultReference
from radardetseval.simulators.DetFromPolyLineSim import DetFromPolyLineSim
from radardetseval.simulators.PolyLineSim import PolyLineSim


def simulate_single_line(point_a: list, point_b: list, n_simulations=10000,
                         data_path=r'private\sim_single_line_out.pickle'):
    print(f'Simulate single lane with:')
    print(f'point_a: {point_a}')
    print(f'point_b: {point_b}')
    print(f'n_simulations: {n_simulations}')
    print(f'data_path: {data_path}')

    ref_pos_std = DefaultReference.pos_std
    ref_vel_std = DefaultReference.vel_std

    meas_cov = np.power(np.diag([DefaultRadar.range_std, DefaultRadar.azimuth_std, DefaultRadar.range_rate_std]), 2.0)
    ref_cov = np.power(np.diag([ref_pos_std, ref_pos_std, ref_vel_std, ref_vel_std]), 2)

    gt_polyline = PolyLineDS()
    gt_polyline.add_point(point_a[0], point_a[1], point_a[2], point_a[3])
    gt_polyline.add_point(point_b[0], point_b[1], point_b[2], point_b[3])

    polyline_simulator = PolyLineSim(gt_polyline)
    det_simulator = DetFromPolyLineSim(gt_polyline)

    sim_out_00 = list()
    sim_out_05 = list()
    sim_out_025 = list()
    sim_out_075 = list()
    sim_out_1 = list()

    for _ in tqdm(range(n_simulations)):
        sim_polyline_cons_dev = polyline_simulator.sim_with_the_same_deviation(ref_cov)

        detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=0.0)
        sim_out_00.append(find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection))

        detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=0.25)
        sim_out_025.append(find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection))

        detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=0.5)
        sim_out_05.append(find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection))

        detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=0.75)
        sim_out_075.append(find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection))

        detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=1.0)
        sim_out_1.append(find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection))

    data = {'gt_polyline': gt_polyline,
            'sim_out_00_df': pd.DataFrame(sim_out_00),
            'sim_out_025_df': pd.DataFrame(sim_out_025),
            'sim_out_05_df': pd.DataFrame(sim_out_05),
            'sim_out_075_df': pd.DataFrame(sim_out_075),
            'sim_out_1_df': pd.DataFrame(sim_out_1),
            'n_simulations': n_simulations}

    with open(data_path, 'wb') as handle:
        pickle.dump(data, handle, protocol=pickle.HIGHEST_PROTOCOL)


if __name__ == '__main__':
    simulate_single_line([4.0, -4.0, 4.0, 0.0],
                         [4.0, 12.0, 4.0, 0.0],
                         100, r'private\sim_single_line_out_vel_4.pickle')