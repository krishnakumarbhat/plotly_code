import pickle
import numpy as np
import pandas as pd
from AptivDataExtractors.utilities.MathFunctions import calc_position_in_bounding_box
from tqdm import tqdm
from radardetseval.association.nearest_det_to_object import find_nearest_det_on_object_fast
from radardetseval.configs.default_radar import DefaultRadar
from radardetseval.configs.defaut_reference import DefaultReference
from radardetseval.simulators.DetFromRadarObjectSim import DetFromRadarObjectSim
from dataclasses import dataclass

from multiprocessing import Pool

from radardetseval.simulators.RadarObjectSim import RadarObjectSim


@dataclass
class Simulator:
    gt_object: pd.Series
    ref_pos_cov: np.array
    ref_vel_cov: np.array
    meas_cov: np.array
    ref_point_x_1: float = 0.0
    ref_point_x_2: float = 1.0
    ref_point_y_1: float = 0.0
    ref_point_y_2: float = 1.0
    meas_bias: np.array = np.zeros(3)

    def __post_init__(self):
        self.object_simulator = RadarObjectSim(self.gt_object)
        self.det_simulator = DetFromRadarObjectSim(self.gt_object)

    def __call__(self, i=1):
        radar_object = self.object_simulator.sim(self.ref_pos_cov, self.ref_vel_cov)
        detection, ref_points_draw = self.det_simulator.sim_single_point_randomly_from_line(self.meas_cov,
                                                                                            self.ref_point_x_1,
                                                                                            self.ref_point_x_2,
                                                                                            self.ref_point_y_1,
                                                                                            self.ref_point_y_2,
                                                                                            self.meas_bias)
        opt_output = find_nearest_det_on_object_fast(radar_object, detection)

        opt_output['gt_ref_point_x'] = ref_points_draw[0]
        opt_output['gt_ref_point_y'] = ref_points_draw[1]
        return opt_output


def simulate_object_from_randomly_from_line(gt_radar_object, meas_bias: np.array = np.zeros(3),
                                            ref_point_x_1=0.0, ref_point_x_2=1.0, ref_point_y_1=0.0, ref_point_y_2=1.0,
                                            n_simulations=10000, data_path=r'private\sim_radar_object_out.pickle',
                                            f_multiprocessing=False, n_pools=6):

    print(f'Simulate single object with:')
    print(f'radar_object: {gt_radar_object}')
    print(f'meas_bias: {meas_bias}')
    print(f'ref_point_x_1: {ref_point_x_1}')
    print(f'ref_point_x_2: {ref_point_x_2}')
    print(f'ref_point_y_1: {ref_point_y_1}')
    print(f'ref_point_y_2: {ref_point_y_2}')
    print(f'n_simulations: {n_simulations}')
    print(f'data_path: {data_path}')

    ref_pos_std = DefaultReference.pos_std
    ref_vel_std = DefaultReference.vel_std

    meas_cov = np.power(np.diag([DefaultRadar.range_std, DefaultRadar.azimuth_std, DefaultRadar.range_rate_std]), 2.0)
    pos_cov = np.power(np.diag([ref_pos_std, ref_pos_std]), 2.0)
    vel_cov = np.power(np.diag([ref_vel_std, ref_vel_std]), 2.0)

    sim = Simulator(gt_radar_object, pos_cov, vel_cov, meas_cov,
                    ref_point_x_1, ref_point_x_2, ref_point_y_1, ref_point_y_2,
                    meas_bias)

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

    data = {'gt_object': gt_radar_object,
            'gt_ref_point_line': {'ref_point_x_1': ref_point_x_1, 'ref_point_x_2': ref_point_x_2,
                                  'ref_point_y_1': ref_point_y_1, 'ref_point_y_2': ref_point_y_2},
            'sim_out': pd.DataFrame(sim_out),
            'n_simulations': n_simulations}

    with open(data_path, 'wb') as handle:
        pickle.dump(data, handle, protocol=pickle.HIGHEST_PROTOCOL)


if __name__ == '__main__':
    gt_object = pd.Series({
        'id': 0,
        'position_x': 6.0,
        'position_y': 5.0,
        'center_x': 6.0,
        'center_y': 5.0,
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
    gt_object['center_x'] = c_x
    gt_object['center_y'] = c_y
    simulate_object_from_randomly_from_line(gt_object, np.zeros(3),
                                            ref_point_x_1=0.0, ref_point_x_2=1.0, ref_point_y_1=0.0, ref_point_y_2=1.0,
                                            n_simulations=2000,
                                            data_path=r'private\single_object_x6_y5_vx4_vy0_ref_point_line01_01_n2000.pickle',
                                            f_multiprocessing=False, n_pools=8)
