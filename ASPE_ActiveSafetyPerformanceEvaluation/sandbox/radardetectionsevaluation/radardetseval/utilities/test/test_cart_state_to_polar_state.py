import numpy as np
import pytest

from radardetseval.utilities.cart_state_to_polar_state import cart_state_to_polar_state

TV = dict()
TV["Front center with radial velocity"] = {
    "x": 2.0,
    "y": 0.0,
    "vx": 2.0,
    "vy": 0.0,
    "exp_range": 2.0,
    "exp_azimuth": np.deg2rad(0),
    "exp_range_rate": 2.0,
    "exp_cross_radial_vel": 0.0
}

TV["Front right with radial velocity"] = {
    "x": 2.0,
    "y": 2.0,
    "vx": -2.0,
    "vy": -2.0,
    "exp_range": 2.8284271247461903,
    "exp_azimuth": np.deg2rad(45),
    "exp_range_rate": -2.8284271247461903,
    "exp_cross_radial_vel": 0.0
}

TV["Front left with radial velocity"] = {
    "x": 2.0,
    "y": -2.0,
    "vx": 2.0,
    "vy": -2.0,
    "exp_range": 2.8284271247461903,
    "exp_azimuth": np.deg2rad(-45),
    "exp_range_rate": 2.8284271247461903,
    "exp_cross_radial_vel": 0.0
}

TV["Front right with cross radial velocity"] = {
    "x": 2.0,
    "y": 2.0,
    "vx": -2.0,
    "vy": 2.0,
    "exp_range": 2.8284271247461903,
    "exp_azimuth": np.deg2rad(45),
    "exp_range_rate": 0.0,
    "exp_cross_radial_vel": 2.8284271247461903
}

TV["Front left with longitudinal velocity"] = {
    "x": 2.0,
    "y": -2.0,
    "vx": 2.0,
    "vy": 0,
    "exp_range": 2.8284271247461903,
    "exp_azimuth": -np.deg2rad(45),
    "exp_range_rate": 1.414213562373095,
    "exp_cross_radial_vel": 1.414213562373095
}

TV["Front right with longitudinal velocity"] = {
    "x": 2.0,
    "y": 2.0,
    "vx": 2.0,
    "vy": 0,
    "exp_range": 2.8284271247461903,
    "exp_azimuth": np.deg2rad(45),
    "exp_range_rate": 1.414213562373095,
    "exp_cross_radial_vel": -1.414213562373095
}

pytest_param_list = list()
for single_tv_name, single_tv in TV.items():
    pytest_param_list.append(pytest.param(single_tv, id=single_tv_name))


class TestCartStateToPolarState:
    """
    Test suite for function cart_state_to_polar_state
    """

    @pytest.mark.parametrize('data', pytest_param_list)
    def test_for_single_values(self, data):
        # Setup
        allowed_diff = 1e-4

        # Evaluate
        range, azimuth, range_rate, cross_radial_vel = \
            cart_state_to_polar_state(data["x"], data["y"], data["vx"], data["vy"])

        # Verify
        assert (range == pytest.approx(data["exp_range"], allowed_diff))
        assert (azimuth == pytest.approx(data["exp_azimuth"], allowed_diff))
        assert (range_rate == pytest.approx(data["exp_range_rate"], allowed_diff))
        assert (cross_radial_vel == pytest.approx(data["exp_cross_radial_vel"], allowed_diff))

