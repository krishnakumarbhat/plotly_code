import numpy as np
from radardetseval.stats.nees import nees_value

from radardetseval.unc_prop.pos_vel_cart_to_polar import unc_prop_pos_rel_vel_cart_to_polar


def nees_function_for_single_det_sim(row, det_cov, ref_cov):
    ref_cov_polar = unc_prop_pos_rel_vel_cart_to_polar(row['sim_ref_x'], row['sim_ref_y'],
                                                       row['sim_ref_vx'], row['sim_ref_vy'],
                                                       ref_cov)
    cov_sum = ref_cov_polar + det_cov

    deviation = np.array([row['sim_dev_range'], row['sim_dev_azimuth'], row['sim_dev_range_rate']])
    nees = nees_value(deviation, cov_sum)
    return nees
