import pytest

from radardetseval.stats.utilities import cdf_to_two_tailed_p_value


@pytest.fixture
def allowed_diff():
    return 1.0e-6


class TestCdfToTwoTailedPValue:

    def test_low_cdf_value(self, allowed_diff):
        cdf_value = 0.1
        expected_p_value = 0.2

        observed = cdf_to_two_tailed_p_value(cdf_value)

        assert (observed == pytest.approx(expected_p_value, allowed_diff))

    def test_big_cdf_value(self, allowed_diff):
        cdf_value = 0.98
        expected_p_value = 0.04

        observed = cdf_to_two_tailed_p_value(cdf_value)

        assert (observed == pytest.approx(expected_p_value, allowed_diff))

    def test_middle_cdf_value(self, allowed_diff):
        cdf_value = 0.5
        expected_p_value = 1

        observed = cdf_to_two_tailed_p_value(cdf_value)

        assert (observed == pytest.approx(expected_p_value, allowed_diff))

