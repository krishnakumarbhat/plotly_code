import numpy as np
import pytest

from aspe.evaluation.RadarDetectionEvaluation.NEES.nees import nees_value


@pytest.fixture
def allowed_diff():
    return 1.0e-6


@pytest.fixture
def diff_2d_small():
    return np.array([1.0, 1.0])


@pytest.fixture
def diff_2d_big_second_quarter():
    return np.array([4.0, -4.0])


@pytest.fixture
def diff_2d_small_and_zero():
    return np.array([0.0, 1.0])


@pytest.fixture
def diff_2d_big_and_zero():
    return np.array([16.0, 0.0])


@pytest.fixture
def diff_2d_several_values():
    return np.array([[1.0, 0.0],
                    [0.0, 1.0],
                    [-1.0, 0.0],
                    [0.0, -1.0]])


@pytest.fixture
def cov_2d_diagonal_small():
    return np.array([[1.0, 0.0], [0.0, 1.0]])


@pytest.fixture
def cov_2d_diagonal_big():
    return np.array([[4.0, 0.0], [0.0, 4.0]])


@pytest.fixture
def cov_2d_cov_small():
    return np.array([[1.0, 0.25], [0.25, 1.0]])


@pytest.fixture
def cov_2d_cov_big_negative():
    return np.array([[1.0, -0.75], [-0.75, 1.0]])


@pytest.fixture
def cov_2d_cov_corr_1():
    return np.array([[1.0, 1.0], [1.0, 1.0]])


# 1d section
@pytest.fixture
def diff_1d_small_value():
    return np.array([1.0])


@pytest.fixture
def diff_1d_several_values():
    return np.array([[1.0],
                    [0.0],
                    [-1.0],
                    [2.0]])


@pytest.fixture
def diff_1d_several_values_wrong_size():
    return np.array([1.0, 0.0, -1.0, 2.0])


@pytest.fixture
def cov_1d_cov_small():
    return np.array([[1.0]])  # note that this is 2d array with shape 1x1


@pytest.fixture
def cov_1d_cov_small_as_1d_array():
    return np.array([1.0])


@pytest.fixture
def deviation_3d():
    return np.array([[[1.0]]])


class TestNEESValue:

    def test_nominal_2d(self, diff_2d_small, cov_2d_diagonal_small, allowed_diff):
        expected_nees_value = 2.0

        observed = nees_value(diff_2d_small, cov_2d_diagonal_small)

        assert (observed == pytest.approx(expected_nees_value, allowed_diff))

    def test_second_quarter_2d(self, diff_2d_big_second_quarter, cov_2d_diagonal_small, allowed_diff):
        expected_nees_value = 32.0

        observed = nees_value(diff_2d_big_second_quarter, cov_2d_diagonal_small)

        assert (observed == pytest.approx(expected_nees_value, allowed_diff))

    def test_second_quarter_big_cov_2d(self, diff_2d_big_second_quarter, cov_2d_diagonal_big, allowed_diff):
        expected_nees_value = 8.0

        observed = nees_value(diff_2d_big_second_quarter, cov_2d_diagonal_big)

        assert (observed == pytest.approx(expected_nees_value, allowed_diff))

    def test_dev_big_and_zero_big_cov_2d(self, diff_2d_big_and_zero, cov_2d_diagonal_big, allowed_diff):
        expected_nees_value = 64.0

        observed = nees_value(diff_2d_big_and_zero, cov_2d_diagonal_big)

        assert (observed == pytest.approx(expected_nees_value, allowed_diff))

    def test_n_samples_2d(self, diff_2d_several_values, cov_2d_diagonal_small, allowed_diff):
        expected_nees_value = np.ones([4])

        observed = nees_value(diff_2d_several_values, cov_2d_diagonal_small)

        assert (np.array_equal(expected_nees_value, observed))

    def test_dev_small_ans_corr_1_2d(self, diff_2d_small, cov_2d_cov_corr_1, allowed_diff):
        expected_nees_value = 1.0

        observed = nees_value(diff_2d_small, cov_2d_cov_corr_1)

        assert (observed == pytest.approx(expected_nees_value, allowed_diff))

    def test_n_samples_with_corr_2d(self, diff_2d_several_values, cov_2d_cov_big_negative, allowed_diff):
        expected_nees_value = np.ones([4]) * 2.28571429

        observed = nees_value(diff_2d_several_values, cov_2d_cov_big_negative)

        assert np.allclose(expected_nees_value, observed, atol=allowed_diff)

    def test_nominal_1d(self, diff_1d_small_value, cov_1d_cov_small, allowed_diff):
        expected_nees_value = 1.0

        observed = nees_value(diff_1d_small_value, cov_1d_cov_small)

        assert (observed == pytest.approx(expected_nees_value, allowed_diff))

    def test_several_values_1d(self, diff_1d_several_values, cov_1d_cov_small, allowed_diff):
        expected_nees_value = np.array([1.0, 0.0, 1.0, 4.0])

        observed = nees_value(diff_1d_several_values, cov_1d_cov_small)

        assert np.allclose(expected_nees_value, observed, atol=allowed_diff)

    def test_nominal_1d_with_1d_cov_array(self, diff_1d_small_value, cov_1d_cov_small_as_1d_array):
        with pytest.raises(ValueError):
            nees_value(diff_1d_small_value, cov_1d_cov_small_as_1d_array)

    def test_nominal_1d_with_wrong_deviation_array(self, diff_1d_several_values_wrong_size, cov_1d_cov_small):
        with pytest.raises(ValueError):
            nees_value(diff_1d_several_values_wrong_size, cov_1d_cov_small)

    def test_1d_cov_with_too_big_deviation_shape(self, deviation_3d, cov_1d_cov_small):
        with pytest.raises(ValueError):
            nees_value(deviation_3d, cov_1d_cov_small)

