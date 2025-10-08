import numpy as np
from scipy.stats import gamma, norm

from aspe.evaluation.RadarObjectsEvaluation.KPI.supporting_functions import cdf_to_two_tailed_p_value


def nees_value(deviation: np.array, cov_matrix: np.array):
    """
    Calculate normalized estimated error square value for given deviation and covariance matrix.
    NEES can be interpreted as normalized deviation by covariance matrix. for one 1-d case it reduces to
    division squared deviation by estimated variance

    In case of normal distribution with corresponding covariance matrix NEES values should follow chi-square
    distribution with k=dimension of random variable X

    Handles N deviation samples, but covariance matrix need to be same for all samples

    NOTE: mahalanobis distance = sqrt(nees_values)

    :param deviation: array of deviation for which nees value should be calculated.
           expected size: [NxK], N - number of samples, K - dimension of deviation variable
                          [K], K - number of samples, in that case single value will be returned

    :param cov_matrix: covariance matrix used for calculating normalized deviation
           expected size: [KxK], K - dimension of deviation variable
                          NOTE: for 1d covariance matrix shape of array needs to be 1x1

    :return: nees values - array of normalized estimated error squared
             expected size: [Nx1], N - number of samples
             expected size: 1, in case of one-dimensional deviation array as an input
    """
    # handling covariance matrix
    if len(cov_matrix.shape) == 2 and cov_matrix.shape[0] == cov_matrix.shape[1]:
        # handle 2d matrix with shape 1,1
        if cov_matrix.shape[0] == 1:
            inv_cov_matrix = 1 / cov_matrix
        else:
            inv_cov_matrix = np.linalg.pinv(cov_matrix)
    else:
        raise ValueError(f'wrong shape of covariance matrix : {cov_matrix.shape}, KxK 2d array allowed')

    # handling 1d and 2d case
    if len(deviation.shape) == 1:
        nees_values = (deviation.transpose() @ inv_cov_matrix) @ deviation

    elif len(deviation.shape) == 2:
        nees_values = np.zeros([deviation.shape[0]])
        for i, _ in enumerate(nees_values):
            single_dev = deviation[i, :]
            nees_values[i] = (single_dev.transpose() @ inv_cov_matrix) @ single_dev

    else:
        raise ValueError(f'wrong shape of deviation array: {deviation.shape}, maximum 2d arrays are supported')

    return nees_values


def nees_test(nees_values: np.array, dimension: int = 1, test_type: str = 'mean Gamma'):
    """
    NEES test is statistical tests to check if NEES values follow chi-square distribution
    Classical version is defined by Yaakov Bar-Shalom and it tests if mean of NEES values follow distribution of
    mean for chi-square distribution which is Gamma distribution.
    For big number of NEES samples nees test can be approximated by normal distribution,
    Type of test can be chosen by user (test_type)

    :param nees_values: nees values - array of normalized estimated error squared
           expected size: [Nx1], N - number of samples

    :param dimension: dimension of variable for which NEES value was calculated, default 1

    :param test_type: type of nees test, default is 'mean Gamma'
    :return:

    references:
    https://en.wikipedia.org/wiki/Chi-squared_distribution (Sample mean)
    """
    if len(nees_values.shape) > 1:
        raise ValueError(f'shape of nees_values is{nees_values.shape}, only 1d arrays are supported')

    if test_type == 'mean Gamma':
        nees_test_variable = nees_values.mean()
        alpha = nees_values.size * dimension / 2.0
        theta = 2.0 / nees_values.size
        gamma_cdf = gamma.cdf(nees_test_variable, a=alpha, scale=theta)
        p_value = cdf_to_two_tailed_p_value(gamma_cdf)

    elif test_type == 'mean Normal':
        nees_test_variable = nees_values.mean()
        expected_nees_mean_sigma = np.sqrt(2.0 * 3.0 / nees_values.size)
        expected_nees_mean = dimension
        # two tailed p-value
        norm_cdf = norm.cdf(nees_test_variable, expected_nees_mean, expected_nees_mean_sigma)
        p_value = cdf_to_two_tailed_p_value(norm_cdf)

    else:
        raise ValueError(f'{test_type} not supported')

    return nees_test_variable, p_value
