import pytest
import numpy as np
from scipy.stats import multivariate_normal
from radardetseval.stats.nees import nees_value, nees_test


@pytest.fixture
def min_allowed_p_value():
    return 1.0e-2


def nees_1d_10_samples():
    np.random.seed(seed=2)
    cov = np.array([[1.0]])
    dev = np.array([multivariate_normal.rvs(np.zeros(1), cov, 10)]).transpose()
    nees = nees_value(dev, cov)
    return nees


def nees_1d_1000_samples():
    np.random.seed(seed=17)
    cov = np.array([[1.0]])
    dev = np.array([multivariate_normal.rvs(np.zeros(1), cov, 1000)]).transpose()
    nees = nees_value(dev, cov)
    return nees


def nees_2d_1_sample():
    np.random.seed(seed=8)
    cov = np.array([[1.0, 0.5], [0.5, 1.0]])
    dev = multivariate_normal.rvs(np.zeros(2), cov, 1)
    nees = nees_value(dev, cov)
    return nees


def nees_2d_3_sample():
    np.random.seed(seed=15)
    cov = np.array([[1.0, 0.5], [0.5, 1.0]])
    dev = multivariate_normal.rvs(np.zeros(2), cov, 3)
    nees = nees_value(dev, cov)
    return nees


def nees_2d_30_samples():
    np.random.seed(seed=7)
    cov = np.array([[1.0, 0.5], [0.5, 1.0]])
    dev = multivariate_normal.rvs(np.zeros(2), cov, 30)
    nees = nees_value(dev, cov)
    return nees


def nees_2d_1000_samples():
    np.random.seed(seed=5)
    cov = np.array([[1.0, -0.1], [-0.1, 1.0]])
    dev = multivariate_normal.rvs(np.zeros(2), cov, 1000)
    nees = nees_value(dev, cov)
    return nees


def nees_2d_100000_samples():
    np.random.seed(seed=5)
    cov = np.array([[1.0, -0.1], [-0.1, 1.0]])
    dev = multivariate_normal.rvs(np.zeros(2), cov, 100000)
    nees = nees_value(dev, cov)
    return nees


def nees_6d_100_samples():
    np.random.seed(seed=124)
    cov = np.diag([1, 2, 3, 4, 5, 6])
    dev = multivariate_normal.rvs(np.zeros(6), cov, 100)
    nees = nees_value(dev, cov)
    return nees


def nees_big_samples():
    pytest_param_list = list()
    pytest_param_list.append(pytest.param(nees_1d_1000_samples(), 1, id='nees_1d_1000_samples'))
    pytest_param_list.append(pytest.param(nees_2d_1000_samples(), 2, id='nees_2d_1000_samples'))
    pytest_param_list.append(pytest.param(nees_2d_100000_samples(), 2, id='nees_2d_100000_samples'))
    pytest_param_list.append(pytest.param(nees_6d_100_samples(), 6, id='nees_6d_100_samples'))
    return pytest_param_list


def nees_medium_samples():
    pytest_param_list = list()
    pytest_param_list.append(pytest.param(nees_1d_10_samples(), 1, id='nees_1d_10_samples'))
    pytest_param_list.append(pytest.param(nees_2d_30_samples(), 2, id='nees_2d_30_samples'))
    return pytest_param_list


def nees_low_samples():
    pytest_param_list = list()
    pytest_param_list.append(pytest.param(nees_2d_1_sample(), 2, id='nees_2d_1_sample'))
    pytest_param_list.append(pytest.param(nees_2d_3_sample(), 2, id='nees_2d_3_sample'))
    return pytest_param_list


def nees_all_samples():
    pytest_param_list = nees_low_samples() + nees_medium_samples() + nees_big_samples()
    return pytest_param_list


@pytest.fixture
def wrong_nees_shape():
    return np.array([[16.0, 1.0], [13.0, 2.0]])


@pytest.fixture
def wrong_test_type():
    return 'mean Gammma'


class TestNEESTest:

    @pytest.mark.parametrize('nees_data, dimension', nees_all_samples())
    def test_all_samples_default_configuration(self, nees_data, dimension, min_allowed_p_value):

        observed_test_variable, observed_p_value = nees_test(nees_data, dimension)

        assert (min_allowed_p_value < observed_p_value)

    @pytest.mark.parametrize('nees_data, dimension', nees_all_samples())
    def test_all_samples_normal_distribution(self, nees_data, dimension, min_allowed_p_value):

        observed_test_variable, observed_p_value = nees_test(nees_data, dimension, test_type='mean Normal')

        assert (min_allowed_p_value < observed_p_value)

    def test_wrong_nees_shape(self, wrong_nees_shape):
        with pytest.raises(ValueError):
            nees_test(wrong_nees_shape, 1)

    def test_wrong_test_type(self, wrong_test_type):
        nees_data = np.array([2.0, 3.0, 5.0])
        with pytest.raises(ValueError):
            nees_test(nees_data, 1, test_type=wrong_test_type)

