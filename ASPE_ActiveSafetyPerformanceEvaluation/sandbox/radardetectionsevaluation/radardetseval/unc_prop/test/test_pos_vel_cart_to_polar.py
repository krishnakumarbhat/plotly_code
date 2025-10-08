import numpy as np
from scipy.stats import multivariate_normal
from radardetseval.unc_prop.pos_vel_cart_to_polar import unc_prop_pos_rel_vel_cart_to_polar
from radardetseval.utilities.cart_state_to_polar_state import cart_state_to_polar_state


class TestPosVelCartToPolar:
    """

    """
    def test_consistency_monte_carlo(self):
        """
        Check uncertainty propagation against monte carlo draw from sample with given covariance matrix
        :return:
        """
        # Configuration
        epsilon = 1.0e-1
        np.random.seed(5)

        pos_std = 1.0
        vel_std = 0.5
        pos_var = pos_std * pos_std
        vel_var = vel_std * vel_std

        cart_cov = np.diag([pos_var, pos_var, vel_var, vel_var])

        x = 10.0
        y = -5.0
        vx = 5.0
        vy = -7.0

        ref_state = np.array([x, y, vx, vy])

        n_samples = 1000

        # Simulate
        state = multivariate_normal.rvs(mean=ref_state, cov=cart_cov, size=n_samples)

        # Calculate reference covariance (from draw)
        det_range, azimuth, range_rate, cross_radial_vel = cart_state_to_polar_state(state[:, 0],
                                                                                     state[:, 1],
                                                                                     state[:, 2],
                                                                                     state[:, 3])

        polar_state = np.array([det_range, azimuth, range_rate])

        polar_cov = np.cov(polar_state)

        # Calculate covariance by linear uncertainty propagation
        polar_cov_estimated = unc_prop_pos_rel_vel_cart_to_polar(x, y, vx, vy, cart_cov)

        assert np.allclose(polar_cov_estimated, polar_cov, atol=epsilon)
