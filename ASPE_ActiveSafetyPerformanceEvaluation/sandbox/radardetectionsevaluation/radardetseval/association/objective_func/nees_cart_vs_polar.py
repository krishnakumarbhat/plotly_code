import numpy as np
from radardetseval.stats.nees import nees_value
from radardetseval.unc_prop.pos_vel_cart_to_polar import unc_prop_pos_rel_vel_cart_to_polar
from radardetseval.utilities.cart_state_to_polar_state import cart_state_to_polar_state


def nees_cart_vs_polar(cart_state, cart_cov, polar_state, polar_cov):

    ref_cov_polar = unc_prop_pos_rel_vel_cart_to_polar(cart_state[0],
                                                       cart_state[1],
                                                       cart_state[2],
                                                       cart_state[3],
                                                       cart_cov)

    cov_sum = ref_cov_polar + polar_cov

    deviation = deviation_cart_vs_polar(cart_state, polar_state)

    nees = nees_value(deviation, cov_sum)
    return nees


def deviation_cart_vs_polar(cart_state, polar_state):
    ref_range, ref_azimuth, ref_range_rate, _ = cart_state_to_polar_state(cart_state[0],
                                                                          cart_state[1],
                                                                          cart_state[2],
                                                                          cart_state[3])
    deviation = np.array([ref_range - polar_state[0],
                          ref_azimuth - polar_state[1],
                          ref_range_rate - polar_state[2]])
    return deviation
