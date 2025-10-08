from radardetseval.Iface.Detection import Detection
from radardetseval.Iface.PolyLineDS import PolyLineDS
import numpy as np
from scipy import optimize
import matplotlib.pyplot as plt

from radardetseval.association.nearest_point_on_line import find_nearest_point_on_segment_discrete, \
    find_nearest_point_on_segment_minimize
from radardetseval.association.objective_func.nees_for_polyline import nees_value_for_vertex_ids, \
    nees_value_for_single_vertex_id
from radardetseval.simulators.DetFromPolyLineSim import DetFromPolyLineSim
from radardetseval.simulators.PolyLineSim import PolyLineSim
from radardetseval.stats.nees import nees_value
from radardetseval.unc_prop.pos_vel_cart_to_polar import unc_prop_pos_rel_vel_cart_to_polar
from radardetseval.utilities.cart_state_to_polar_state import cart_state_to_polar_state


def nees_function(row, single_det: Detection):
    cart_state = np.array([row['x'], row['y'], row['vx'], row['vy']])
    cart_cov = np.array(row['cov_matrix'])
    ref_range, ref_azimuth, ref_range_rate, ref_cross_radial_vel = \
        cart_state_to_polar_state(cart_state[0],
                                  cart_state[1],
                                  cart_state[2],
                                  cart_state[3])
    ref_cov_polar = unc_prop_pos_rel_vel_cart_to_polar(row['x'], row['y'], row['vx'], row['vy'], cart_cov)
    det_cov = np.diag([single_det.det_range_var, single_det.det_azimuth_var, single_det.det_range_rate_var])
    cov_sum = ref_cov_polar + det_cov

    deviation = np.array([ref_range - single_det.det_range,
                          ref_azimuth - single_det.det_azimuth,
                          ref_range_rate - single_det.det_range_rate])

    nees = nees_value(deviation, cov_sum)
    return nees


ref_pos_std = 0.05
ref_vel_std = 0.1

range_std = 0.2
azimuth_std = np.deg2rad(0.3)
range_rate_std = 0.06

dl = 0.01

meas_cov = np.power(np.diag([range_std, azimuth_std, range_rate_std]), 2.0)
ref_cov = np.power(np.diag([ref_pos_std, ref_pos_std, ref_vel_std, ref_vel_std]), 2)


gt_polyline = PolyLineDS()
gt_polyline.add_point(4, -4, 4, 0)
gt_polyline.add_point(4, 12, 4, 0)

polyline_simulator = PolyLineSim(gt_polyline)

sim_polyline_cons_dev = polyline_simulator.sim_with_the_same_deviation(ref_cov)
sim_polyline_diff_dev = polyline_simulator.sim_with_the_different_deviations(ref_cov)

discrete_signals_sim = gt_polyline.discretize_single_polygon(sim_polyline_cons_dev.signals, dl)

det_simulator = DetFromPolyLineSim(gt_polyline)

detection = det_simulator.sim_single_point_from_vertex(meas_cov, vertex_id=1.0)

discrete_signals_sim['nees_values'] = discrete_signals_sim.apply(nees_function, axis=1, args=[detection])

# Find nearest point using association function
nearest_point = find_nearest_point_on_segment_discrete(sim_polyline_cons_dev, detection, dl)

# Find by minimizing
vertex_ids = sim_polyline_cons_dev.signals['vertex_id'].values
min_max = (vertex_ids.min(), vertex_ids.max())
stat_vertex = np.array([0.99])

single_nees = nees_value_for_vertex_ids(np.array([1.0]), sim_polyline_cons_dev, detection)

opt_out = optimize.minimize(nees_value_for_vertex_ids,
                            stat_vertex,
                            args=(sim_polyline_cons_dev, detection),
                            bounds=[min_max])

opt_out_scalar = optimize.minimize_scalar(nees_value_for_single_vertex_id,
                                          args=(sim_polyline_cons_dev, detection),
                                          bounds=min_max,
                                          method='Bounded')

# And with wrapper function
nearest_point_min = find_nearest_point_on_segment_minimize(sim_polyline_cons_dev, detection)

# Check function around min
delta_x = 0.01
n_point = 1000

close_vertexes = np.linspace(nearest_point_min.vertex_id - delta_x,
                             nearest_point_min.vertex_id + delta_x,
                             n_point)

close_values_nees = nees_value_for_vertex_ids(close_vertexes, sim_polyline_cons_dev, detection)

fig1, ax1 = plt.subplots()
ax1.plot(close_vertexes, close_values_nees)
