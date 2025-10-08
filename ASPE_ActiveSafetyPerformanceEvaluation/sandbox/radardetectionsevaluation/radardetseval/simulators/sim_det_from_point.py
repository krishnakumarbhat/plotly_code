import numpy as np
from scipy.stats import multivariate_normal

from radardetseval.Iface.Detection import Detection
from radardetseval.Iface.SinglePointSimOut import SinglePointSimOut
from radardetseval.utilities.cart_state_to_polar_state import cart_state_to_polar_state


def sim_det_from_scs_cart_point(gt_state_cart: np.array, ref_cov: np.array, meas_cov: np.array, n_samples=1):
    """
    Function to simulate single detections from single reference detection. reference and measurement covariances
    need to be defined

    :param gt_state_cart: reference state: [x, y, vx, vy] - 1d array
    :param ref_cov: covariance matrix for corresponding state
    :param meas_cov: covariance matrix for measurement (should be diagonal)
    :param n_samples: number of samples of samples to be draw, default 1
    :return:
    """
    # Simulate reference
    sim_ref_state_cart = multivariate_normal.rvs(mean=gt_state_cart, cov=ref_cov, size=n_samples)

    # In case of 1d case increase size to handle further processing
    if n_samples == 1:
        sim_ref_state_cart = np.array([sim_ref_state_cart])

    sim_ref_range, sim_ref_azimuth, sim_ref_range_rate, sim_ref_cross_radial_vel = \
        cart_state_to_polar_state(sim_ref_state_cart[:, 0],
                                  sim_ref_state_cart[:, 1],
                                  sim_ref_state_cart[:, 2],
                                  sim_ref_state_cart[:, 3])

    ref_range, ref_azimuth, ref_range_rate, ref_cross_radial_vel = \
        cart_state_to_polar_state(gt_state_cart[0],
                                  gt_state_cart[1],
                                  gt_state_cart[2],
                                  gt_state_cart[3])

    ref_state_polar = np.array([ref_range, ref_azimuth, ref_range_rate])

    sim_measurement = multivariate_normal.rvs(mean=ref_state_polar, cov=meas_cov, size=n_samples)

    # In case of 1d case increase size to handle further processing
    if n_samples == 1:
        sim_measurement = np.array([sim_measurement])

    deviation_range = sim_measurement[:, 0] - sim_ref_range
    deviation_azimuth = sim_measurement[:, 1] - sim_ref_azimuth
    deviation_range_rate = sim_measurement[:, 2] - sim_ref_range_rate

    # Populate outputs
    outputs = SinglePointSimOut()
    outputs.gt_x = gt_state_cart[0]
    outputs.gt_y = gt_state_cart[1]
    outputs.gt_vx = gt_state_cart[2]
    outputs.gt_vy = gt_state_cart[3]
    outputs.gt_range = ref_range
    outputs.gt_azimuth = ref_azimuth
    outputs.gt_range_rate = ref_range_rate
    outputs.gt_cross_radial_vel = ref_cross_radial_vel
    outputs.ref_cov = ref_cov
    outputs.meas_cov = meas_cov

    outputs.signals["sim_ref_x"] = sim_ref_state_cart[:, 0]
    outputs.signals["sim_ref_y"] = sim_ref_state_cart[:, 1]
    outputs.signals["sim_ref_vx"] = sim_ref_state_cart[:, 2]
    outputs.signals["sim_ref_vy"] = sim_ref_state_cart[:, 3]

    outputs.signals["sim_ref_range"] = sim_ref_range
    outputs.signals["sim_ref_azimuth"] = sim_ref_azimuth
    outputs.signals["sim_ref_range_rate"] = sim_ref_range_rate
    outputs.signals["sim_ref_cross_radial_vel"] = sim_ref_cross_radial_vel

    outputs.signals["sim_meas_range"] = sim_measurement[:, 0]
    outputs.signals["sim_meas_azimuth"] = sim_measurement[:, 1]
    outputs.signals["sim_meas_range_rate"] = sim_measurement[:, 2]

    outputs.signals["sim_dev_range"] = deviation_range
    outputs.signals["sim_dev_azimuth"] = deviation_azimuth
    outputs.signals["sim_dev_range_rate"] = deviation_range_rate

    return outputs


def sim_single_det_from_scs_cart_point(gt_state_cart: np.array, meas_cov: np.array,
                                       meas_bias: np.array = np.zeros(3)):
    """
    Function to simulate single detections from single reference detection. reference and measurement covariances
    need to be defined

    :param gt_state_cart: ground truth state: [x, y, vx_rel, vy_rel] - 1d array
    :param meas_cov: covariance matrix for measurement (probably be diagonal)
    :param meas_bias: bias(optional) bias array for measurement - optional, default np.zeros(3)
    :return:
    """

    ref_range, ref_azimuth, ref_range_rate, ref_cross_radial_vel = \
        cart_state_to_polar_state(gt_state_cart[0],
                                  gt_state_cart[1],
                                  gt_state_cart[2],
                                  gt_state_cart[3])

    ref_state_polar = np.array([ref_range, ref_azimuth, ref_range_rate])

    sim_measurement = multivariate_normal.rvs(mean=ref_state_polar, cov=meas_cov, size=1) + meas_bias

    # Populate outputs
    outputs = Detection(sim_measurement[0], sim_measurement[1], sim_measurement[2],
                        meas_cov[0, 0], meas_cov[1, 1], meas_cov[2, 2])

    return outputs


