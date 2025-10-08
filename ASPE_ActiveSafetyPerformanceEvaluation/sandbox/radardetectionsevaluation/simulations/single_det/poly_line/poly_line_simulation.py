import pickle
import numpy as np
import pandas as pd
from tqdm import tqdm
from radardetseval.Iface.PolyLineDS import PolyLineDS
from radardetseval.association.nearest_point_on_line import find_nearest_point_on_segment_minimize
from radardetseval.simulators.DetFromPolyLineSim import DetFromPolyLineSim
from radardetseval.simulators.PolyLineSim import PolyLineSim


data_path = r'private\sim_rectangle_polyline_vel_4.pickle'

ref_pos_std = 0.05
ref_vel_std = 0.1

range_std = 0.2
azimuth_std = np.deg2rad(0.3)
range_rate_std = 0.06

meas_cov = np.power(np.diag([range_std, azimuth_std, range_rate_std]), 2.0)
ref_cov = np.power(np.diag([ref_pos_std, ref_pos_std, ref_vel_std, ref_vel_std]), 2)


gt_polyline = PolyLineDS()
gt_polyline.add_point(4.0, -4.0, 4.0, 0.0)
gt_polyline.add_point(4.0, 4.0, 4.0, 0.0)
gt_polyline.add_point(6.0, 4.0, 4.0, 0.0)
gt_polyline.add_point(6.0, -4.0, 4.0, 0.0)
gt_polyline.add_point(4.0, -4.0, 4.0, 0.0)

polyline_simulator = PolyLineSim(gt_polyline)
det_simulator = DetFromPolyLineSim(gt_polyline)

n_simulations = 100

sim_out_0 = list()
sim_out_1 = list()
sim_out_2 = list()
sim_out_3 = list()
sim_out_4 = list()

for i in tqdm(range(n_simulations)):
    sim_polyline_cons_dev = polyline_simulator.sim_with_the_same_deviation(ref_cov)

    detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=0.0)
    sim_out_0.append(find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection))

    detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=1.0)
    sim_out_1.append(find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection))

    detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=2.0)
    sim_out_2.append(find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection))

    detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=3.0)
    sim_out_3.append(find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection))

    detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=4.0)
    sim_out_4.append(find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection))

data = {'gt_polyline': gt_polyline,
        'sim_out_0_df': pd.DataFrame(sim_out_0),
        'sim_out_1_df': pd.DataFrame(sim_out_1),
        'sim_out_2_df': pd.DataFrame(sim_out_2),
        'sim_out_3_df': pd.DataFrame(sim_out_3),
        'sim_out_4_df': pd.DataFrame(sim_out_4)}


with open(data_path, 'wb') as handle:
    pickle.dump(data, handle, protocol=pickle.HIGHEST_PROTOCOL)


