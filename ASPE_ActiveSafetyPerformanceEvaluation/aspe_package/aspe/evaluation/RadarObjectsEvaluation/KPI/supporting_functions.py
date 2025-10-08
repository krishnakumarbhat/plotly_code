import numpy as np


def cdf_to_two_tailed_p_value(cdf_value):
    """
    Calculates two tailed p-value for given cumulative distribution function value for test variable.
    :param cdf_value: cumulative distribution function value for test variable
    :return: p-value with the same shape as cdf_value
    """
    return 1 - np.abs(-2 * cdf_value + 1)
