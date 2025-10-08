import numpy as np
import uncertainties
from uncertainties import ufloat
import matplotlib.pyplot as plt

from radardetseval.simulators.sim_det_from_point import sim_det_from_scs_cart_point
from radardetseval.unc_prop.u_cart_state_to_polar_state import u_cart_state_to_polar_state


def nees_function(row, ness_cov_inv):
    deviation = np.array([row['deviation_range'], row['deviation_azimuth'], row['deviation_range_rate']])
    nees = (deviation.transpose() @ ness_cov_inv) @ deviation
    return nees


range_std = 0.2
azimuth_std = np.deg2rad(0.3)
range_rate_std = 0.06

ref_x = 5.0
ref_y = 1.0
ref_vx = -2.0
ref_vy = 3.0

ref_pos_std = 0.05

ref_vel_std = 0.1

ref_cov = np.power(np.diag([ref_pos_std, ref_pos_std, ref_vel_std, ref_vel_std]), 2.0)
meas_cov = np.power(np.diag([range_std, azimuth_std, range_rate_std]), 2.0)
ref_state_cart = np.array([ref_x, ref_y, ref_vx, ref_vy])
N_samples = 100000

sim_out = sim_det_from_scs_cart_point(ref_state_cart, ref_cov, meas_cov, N_samples)

# Calculate errors
sim_out.signals['deviation_range'] = sim_out.signals.sim_ref_range - sim_out.signals.sim_meas_range
sim_out.signals['deviation_azimuth'] = sim_out.signals.sim_ref_azimuth - sim_out.signals.sim_meas_azimuth
sim_out.signals['deviation_range_rate'] = sim_out.signals.sim_ref_range_rate - sim_out.signals.sim_meas_range_rate

# Propagate uncertainty from real data

u_ref_x = ufloat(ref_x, ref_pos_std)
u_ref_y = ufloat(ref_y, ref_pos_std)
u_ref_vx = ufloat(ref_vx, ref_vel_std)
u_ref_vy = ufloat(ref_vy, ref_vel_std)

range, azimuth, range_rate, cross_radial_vel = u_cart_state_to_polar_state(u_ref_x,
                                                                           u_ref_y,
                                                                           u_ref_vx,
                                                                           u_ref_vy)

ref_cov_pol = np.array(uncertainties.covariance_matrix([range, azimuth, range_rate]))

nees_cov = ref_cov_pol + meas_cov
meas_cov_inv = np.linalg.inv(meas_cov)
ness_cov_inv_value = np.linalg.inv(nees_cov)

sim_out.signals['nees_values'] = sim_out.signals.apply(nees_function, axis=1, args=[ness_cov_inv_value])
sim_out.signals['nees_values_meas_cov'] = sim_out.signals.apply(nees_function, axis=1, args=[meas_cov_inv])

nees_mean = sim_out.signals['nees_values'].mean()
nees_mean_var = 2 * 3.0 / N_samples
nees_test_variable = (nees_mean - 3.0) / np.sqrt(nees_mean_var)

nees_meas_cov_mean = sim_out.signals['nees_values_meas_cov'].mean()
nees_meas_cov_mean_var = 2 * 3.0 / N_samples
nees_meas_cov_test_variable = (nees_meas_cov_mean - 3.0) / np.sqrt(nees_meas_cov_mean_var)

fig_1 = plt.figure()
sim_out.signals['nees_values'].hist(bins=100)
fig_2 = plt.figure()
sim_out.signals['nees_values_meas_cov'].hist(bins=100)

