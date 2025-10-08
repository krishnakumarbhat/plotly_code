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
from dataclasses import dataclass

from multiprocessing import Pool


@dataclass
class Simulator:
    gt_polyline: PolyLineDS
    meas_cov: np.array
    ref_cov: np.array

    def __post_init__(self):
        self.polyline_simulator = PolyLineSim(self.gt_polyline)
        self.det_simulator = DetFromPolyLineSim(self.gt_polyline)

    def __call__(self, i=0):
        sim_polyline_cons_dev = self.polyline_simulator.sim_with_the_same_deviation(self.ref_cov)
        detection, vertex_id_draw = self.det_simulator.sim_single_point_randomly(self.meas_cov)
        nearest_det = find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection)
        nearest_det.drop('cov_matrix', inplace=True)
        nearest_det['gt_vertex_id'] = vertex_id_draw

        return nearest_det


def simulate_single_line_randomly(point_a: list, point_b: list, n_simulations=10000,
                                  data_path=r'private\sim_single_line_out.pickle', f_multiprocessing=False):
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

    sim = Simulator(gt_polyline, meas_cov, ref_cov)

    data_range = range(n_simulations)
    sim_out = list()

    if f_multiprocessing:
        with Pool(6) as pool:
            for result in tqdm(pool.imap_unordered(sim, data_range), total=n_simulations):
                sim_out.append(result)
    else:
        for _ in tqdm(data_range):
            single_out = sim()
            sim_out.append(single_out)

    data = {'gt_polyline': gt_polyline,
            'sim_out': pd.DataFrame(sim_out),
            'n_simulations': n_simulations}

    with open(data_path, 'wb') as handle:
        pickle.dump(data, handle, protocol=pickle.HIGHEST_PROTOCOL)


if __name__ == '__main__':
    simulate_single_line_randomly([4.0, -4.0, 4.0, 0.0],
                                  [4.0, -3.0, 4.0, 0.0],
                                  250000, r'private\sim_single_line_randomly_out_vel_4_250000.pickle', True)
