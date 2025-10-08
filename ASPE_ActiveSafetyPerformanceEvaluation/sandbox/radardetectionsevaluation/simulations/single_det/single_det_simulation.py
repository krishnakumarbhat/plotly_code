import pickle
import numpy as np
from radardetseval.configs.default_radar import DefaultRadar
from radardetseval.configs.defaut_reference import DefaultReference
from radardetseval.simulators.sim_det_from_point import sim_det_from_scs_cart_point
from radardetseval.utilities.nees_support import nees_function_for_single_det_sim


def simulate_single_point(gt_state_cart: list, n_simulations=10000,
                          data_path=r'private\sim_single_point_out.pickle'):

    ref_pos_std = DefaultReference.pos_std
    ref_vel_std = DefaultReference.vel_std

    ref_cov = np.power(np.diag([ref_pos_std, ref_pos_std, ref_vel_std, ref_vel_std]), 2.0)
    meas_cov = np.power(np.diag([DefaultRadar.range_std, DefaultRadar.azimuth_std, DefaultRadar.range_rate_std]), 2.0)

    # Simulation
    sim_out = sim_det_from_scs_cart_point(gt_state_cart, ref_cov, meas_cov, n_simulations)

    ref_cov_zero = np.zeros_like(ref_cov)
    sim_out.signals['nees_values'] = sim_out.signals.apply(nees_function_for_single_det_sim, axis=1,
                                                           args=[sim_out.meas_cov, ref_cov_zero])

    with open(data_path, 'wb') as handle:
        pickle.dump(sim_out, handle, protocol=pickle.HIGHEST_PROTOCOL)


if __name__ == '__main__':
    simulate_single_point([4.0, -4.0, 4.0, 0.0],
                          100000, r'private\sim_single_point_x4_00_y4_00_vx4_00_vy_00_no_ref_cov.pickle')