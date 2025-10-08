import numpy as np
import pandas as pd
from scipy import optimize
from radardetseval.Iface.Detection import Detection
from radardetseval.Iface.PolyLineDS import PolyLineDS
from radardetseval.association.objective_func.nees_for_polyline import nees_value_for_single_vertex_id
from radardetseval.stats.nees import nees_value
from radardetseval.unc_prop.pos_vel_cart_to_polar import unc_prop_pos_rel_vel_cart_to_polar
from radardetseval.utilities.cart_state_to_polar_state import cart_state_to_polar_state


def find_nearest_point_on_segment_discrete(polyline: PolyLineDS, det: Detection, accuracy=1.0e-3):

    def nees_function(row, single_det: Detection):
        cart_state = np.array([row['x'], row['y'], row['vx'], row['vy']])
        cart_cov = row['cov_matrix']
        local_ref_range, local_ref_azimuth, local_ref_range_rate, ref_cross_radial_vel = \
            cart_state_to_polar_state(cart_state[0],
                                      cart_state[1],
                                      cart_state[2],
                                      cart_state[3])
        ref_cov_polar = unc_prop_pos_rel_vel_cart_to_polar(row['x'], row['y'], row['vx'], row['vy'], cart_cov)
        det_cov = np.diag([single_det.det_range_var, single_det.det_azimuth_var, single_det.det_range_rate_var])
        cov_sum = ref_cov_polar + det_cov

        deviation = np.array([local_ref_range - single_det.det_range,
                              local_ref_azimuth - single_det.det_azimuth,
                              local_ref_range_rate - single_det.det_range_rate])

        nees = nees_value(deviation, cov_sum)
        return nees

    discrete_signals = polyline.discretize_single_polygon(polyline.signals, accuracy)
    nees_values = discrete_signals.apply(nees_function, axis=1, args=[det])
    idx_min = nees_values.idxmin()
    point_ref = discrete_signals.iloc[idx_min, :]
    ref_range, ref_azimuth, ref_range_rate, _ = cart_state_to_polar_state(point_ref['x'],
                                                                          point_ref['y'],
                                                                          point_ref['vx'],
                                                                          point_ref['vy'])
    det_series = pd.Series([ref_range, ref_azimuth, ref_range_rate, nees_values[idx_min]],
                           ['range', 'azimuth', 'range_rate', 'nees_value'])

    output = point_ref.append(det_series)
    return output


def find_nearest_point_on_segment_minimize(polyline: PolyLineDS, detection: Detection):
    vertex_ids = polyline.signals['vertex_id'].values
    min_max = (vertex_ids.min(), vertex_ids.max())
    opt_out = optimize.minimize_scalar(nees_value_for_single_vertex_id,
                                       args=(polyline, detection),
                                       bounds=min_max,
                                       method='Bounded')

    # get output
    output = get_output_series(polyline, opt_out)
    return output


def find_nearest_point_on_polyline_minimize(polyline: PolyLineDS, detection: Detection):
    vertex_ids = polyline.signals['vertex_id'].values

    opt_series_list = list()
    for i in range(len(vertex_ids)-1):
        min_max = (vertex_ids[i], vertex_ids[i+1])
        single_opt_out = optimize.minimize_scalar(nees_value_for_single_vertex_id,
                                           args=(polyline, detection),
                                           bounds=min_max,
                                           method='Bounded')
        opt_series_list.append(pd.Series(single_opt_out))

    opt_df = pd.DataFrame(opt_series_list)
    fun_series = opt_df.fun
    idx_min = fun_series.idxmin(fun_series)
    opt_out = opt_df.iloc[idx_min, :]

    # get output
    output = get_output_series(polyline, opt_out)
    return output


def get_output_series(polyline, opt_out):
    vertex_ids = polyline.signals['vertex_id'].values
    single_point_df = polyline.interpolate_polygon(np.array([opt_out.x]), vertex_ids, polyline.signals)
    point_ref = single_point_df.iloc[0, :]
    ref_range, ref_azimuth, ref_range_rate, _ = cart_state_to_polar_state(point_ref['x'],
                                                                          point_ref['y'],
                                                                          point_ref['vx'],
                                                                          point_ref['vy'])
    ref_det_series = pd.Series([ref_range, ref_azimuth, ref_range_rate, opt_out.fun],
                               ['range', 'azimuth', 'range_rate', 'nees_value'])

    output = point_ref.append(ref_det_series)
    return output


