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
        nearest_det = find_nearest_point_on_polyline_minimize(sim_polyline_cons_dev, detection)
        nearest_det.drop('cov_matrix', inplace=True)
        nearest_det['gt_vertex_id'] = vertex_id_draw

        return nearest_det


def simulate_arc(arc_vertex: list, segments_length: float,
                 first_segment_orientation=np.deg2rad(90.0), arc=np.deg2rad(90.0),
                 n_simulations=10000, data_path=r'private\sim_three_points_randomly_out.pickle',
                 f_multiprocessing=False, n_pools=6):

    print(f'Simulate single lane with:')
    print(f'arc_vertex: {arc_vertex}')
    print(f'segments_length: {segments_length}')
    print(f'arc [deg]: {np.rad2deg(arc)}')
    print(f'n_simulations: {n_simulations}')
    print(f'data_path: {data_path}')

    ref_pos_std = DefaultReference.pos_std
    ref_vel_std = DefaultReference.vel_std

    meas_cov = np.power(np.diag([DefaultRadar.range_std, DefaultRadar.azimuth_std, DefaultRadar.range_rate_std]), 2.0)
    ref_cov = np.power(np.diag([ref_pos_std, ref_pos_std, ref_vel_std, ref_vel_std]), 2)

    first_point_x = arc_vertex[0] + segments_length * np.cos(first_segment_orientation)
    first_point_y = arc_vertex[1] + segments_length * np.sin(first_segment_orientation)

    third_point_x = arc_vertex[0] + segments_length * np.cos(first_segment_orientation + arc)
    third_point_y = arc_vertex[1] + segments_length * np.sin(first_segment_orientation + arc)

    gt_polyline = PolyLineDS()
    gt_polyline.add_point(first_point_x, first_point_y, arc_vertex[2], arc_vertex[3])
    gt_polyline.add_point(arc_vertex[0], arc_vertex[1], arc_vertex[2], arc_vertex[3])
    gt_polyline.add_point(third_point_x, third_point_y, arc_vertex[2], arc_vertex[3])

    sim = Simulator(gt_polyline, meas_cov, ref_cov)

    data_range = range(n_simulations)
    sim_out = list()

    if f_multiprocessing:
        with Pool(n_pools) as pool:
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
    simulate_arc([4.0, -3.0, 4.0, 0.0], 1.0,
                 np.deg2rad(90.0), np.deg2rad(-45.0),
                 2000, r'private\sim_arc_45_degree_vel_4_2000.pickle', True)
