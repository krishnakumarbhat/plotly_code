import math
import pytest
import numpy as np
import pandas as pd

from aspe.evaluation.RadarDetectionEvaluation.Utilities.azimuth_deviation import \
    calc_azimuth_deviation_for_given_rr_and_vel

test_data = [
    pytest.param(np.deg2rad(0.0), np.deg2rad(0.0), 10.0, 10.0, 0.0, id='azimuth 0 degree, deviation 0 degree'),
    pytest.param(np.deg2rad(0.0), np.deg2rad(0.0), 10.0, 10.0, 10.0, id='azimuth 0 degree, vel not aligned deviation 0 degree'),
    pytest.param(np.deg2rad(90.0), np.deg2rad(0.0), 10.0, 10.0, 10.0, id='azimuth 90 degree, vel not aligned deviation 0 degree'),
    pytest.param(np.deg2rad(90.0), np.deg2rad(0.0), 10.0, 0.0, 10.0, id='azimuth 90 degree, deviation 0 degree'),
    pytest.param(np.deg2rad(75.0), np.deg2rad(15.0), 10.0, 0.0, 10.0, id='azimuth 90 degree, deviation 15 degree'),
    pytest.param(np.deg2rad(120.0), np.deg2rad(-30.0), 10.0, 0.0, 10.0, id='azimuth 90 degree, deviation -30 degree'),
    pytest.param(np.deg2rad(-120.0), np.deg2rad(120.0), 10.0, 10.0, 10.0, id='azimuth 90 degree, deviation -120 degree'),
    pytest.param(np.deg2rad(-150.0), np.deg2rad(-120.0), 10.0, 10.0, 10.0, id='azimuth 90 degree, deviation -120 degree'),
    pytest.param(np.deg2rad(135.0), np.deg2rad(0.0), 0.0, 10.0, 10.0, id='zero range, azimuth 135 degree, deviation 0 degree')
]


@pytest.fixture()
def accepted_error():
    return np.deg2rad(0.01)


class TestCalcAzimuthDeviationForGivenRrAndVel:
    @pytest.mark.parametrize('azimuth, expected_deviation, range_rate, vx, vy', test_data)
    def test_nominal(self, azimuth, expected_deviation, range_rate, vx, vy, accepted_error):
        azimuth_deviation = calc_azimuth_deviation_for_given_rr_and_vel(azimuth, range_rate, vx, vy)
        assert azimuth_deviation == pytest.approx(expected_deviation, accepted_error)

    def test_nominal_np(self, accepted_error):
        azimuth = np.array([np.deg2rad(0.0), np.deg2rad(75.0), np.deg2rad(-120.0)])
        expected_deviation = np.array([np.deg2rad(0.0), np.deg2rad(15.0), np.deg2rad(120.0)])
        range_rate = np.array([10.0, 5.0, 3.0])
        vx = np.array([10.0, 0.0, 3.0])
        vy = np.array([0.0, 5.0, 3.0])
        azimuth_deviation = calc_azimuth_deviation_for_given_rr_and_vel(azimuth, range_rate, vx, vy)
        np.testing.assert_array_almost_equal(azimuth_deviation, expected_deviation, 4)

    def test_nominal_series(self, accepted_error):
        azimuth = pd.Series([np.deg2rad(0.0), np.deg2rad(75.0), np.deg2rad(-120.0)])
        expected_deviation = np.array([np.deg2rad(0.0), np.deg2rad(15.0), np.deg2rad(120.0)])
        range_rate = pd.Series([10.0, 5.0, 3.0])
        vx = pd.Series([10.0, 0.0, 3.0])
        vy = pd.Series([0.0, 5.0, 3.0])
        azimuth_deviation = calc_azimuth_deviation_for_given_rr_and_vel(azimuth, range_rate, vx, vy)
        np.testing.assert_array_almost_equal(azimuth_deviation, expected_deviation, 4)

    def test_zero_vel(self):
        azimuth = 0.0
        range_rate = 10.0
        vx = 0.0
        vy = 0.0
        azimuth_deviation = calc_azimuth_deviation_for_given_rr_and_vel(azimuth, range_rate, vx, vy)

        assert math.isnan(azimuth_deviation)

    def test_rr_above_vel_magnitude(self):
        azimuth = 0.0
        range_rate = 10.0
        vx = 1.0
        vy = 1.0
        azimuth_deviation = calc_azimuth_deviation_for_given_rr_and_vel(azimuth, range_rate, vx, vy)

        assert math.isnan(azimuth_deviation)
