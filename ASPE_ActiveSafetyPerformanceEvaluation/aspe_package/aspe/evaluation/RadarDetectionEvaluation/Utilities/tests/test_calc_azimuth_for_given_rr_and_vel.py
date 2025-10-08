import math
import pytest

from aspe.evaluation.RadarDetectionEvaluation.Utilities.azimuth_deviation import \
    calc_azimuth_for_given_rr_and_vel

test_data = [
    pytest.param(10.0, 10.0, 0.0, id='expected azimuth 0 degree'),
    pytest.param(10.0, 0.0, 10.0, id='expected azimuth 90 degree'),
    pytest.param(10.0, 10.0, 10.0, id='expected azimuth 45 degree'),
    pytest.param(5.0, 8.0, 11.0, id='expected azimuth random degree'),
    pytest.param(0.0, 10.0, 10.0, id='zero range rate')
]


def verify_rr(vx, vy, azimuth, expected_rr):

    observed_rr = math.cos(azimuth) * vx + math.sin(azimuth) * vy

    assert expected_rr == pytest.approx(observed_rr, 0.01)


class TestCalcAzimuthForGivenRrAndVel:
    @pytest.mark.parametrize('range_rate, vx, vy', test_data)
    def test_nominal_azimuth_positive(self, range_rate, vx, vy):

        azimuth_pos, _ = calc_azimuth_for_given_rr_and_vel(range_rate, vx, vy)
        verify_rr(vx, vy, azimuth_pos, range_rate)

    @pytest.mark.parametrize('range_rate, vx, vy', test_data)
    def test_nominal_azimuth_negative(self, range_rate, vx, vy):

        _, azimuth_neg = calc_azimuth_for_given_rr_and_vel(range_rate, vx, vy)
        verify_rr(vx, vy, azimuth_neg, range_rate)

    def test_zero_vel(self):
        range_rate = 10.0
        vx = 0.0
        vy = 0.0
        azimuth_pos, azimuth_neg = calc_azimuth_for_given_rr_and_vel(range_rate, vx, vy)

        assert math.isnan(azimuth_pos)
        assert math.isnan(azimuth_neg)

    def test_rr_above_vel_magnitude(self):
        range_rate = 10.0
        vx = 1.0
        vy = 1.0
        azimuth_pos, azimuth_neg = calc_azimuth_for_given_rr_and_vel(range_rate, vx, vy)

        assert math.isnan(azimuth_pos)
        assert math.isnan(azimuth_neg)

