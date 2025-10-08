# coding=utf-8
"""
Supporting functions for uncertainty propagation
"""
import numpy as np


def propagate_uncertainty_2d(var_a, var_b, cov_a_b, j_11, j_12, j_21, j_22):
    """
    Calculates covariance matrix values using uncertainty propagation by linearization using Jacobian matrix.
    Accepts series of data (e.g. np.array or pd.Series). Internally uses numpy matrix multiplication.

    Let Sigma be the covariance matrix for signals A and B.
    Let f be a mapping from 2-D space to 2-D space: (C,D) = f(A,B).
    Let J = [[j_11, j_12], [j_21, j_22]] be the Jacobian matrix of the transformation f,
    that is: j_11 = dC/dA, j_12 = dC/dB, j_21 = dD/dA, j_22 = dD/dB.
    Given the values of j_11, j_12, j_21, j_22, one can calculate the covariance matrix
    for signals C and D using the formula: Sigma_f = J @ Sigma @ J^T.

    See https://en.wikipedia.org/wiki/Propagation_of_uncertainty#Non-linear_combinations for details.

    :param var_a: variance(s) of A
    :param var_b: variance(s) of B
    :param cov_a_b: covariance(s) of A and B
    :param j_11: top-left element(s) of Jacobian matrix
    :param j_12: top-right element(s) of Jacobian matrix
    :param j_21: bottom-left element(s) of Jacobian matrix
    :param j_22: bottom-right element(s) of Jacobian matrix
    :return: a triple of (variance(s) of C, variance(s) of D, covariance(s) of C and D)
    """
    jacobian = np.array([
        [j_11, j_12],
        [j_21, j_22],
    ]).transpose((2, 0, 1))  # changes indexing from [i, j, row] to [row, i, j]

    sigma = np.array([
        [var_a, cov_a_b],
        [cov_a_b, var_b],
    ]).transpose((2, 0, 1))  # changes indexing from [i, j, row] to [row, i, j]

    jacobian_t = jacobian.transpose((0, 2, 1))  # changes indexing from [row, i, j] to [row, j, i]

    # performs one-by-one multiplication of 2x2 matrices (see np.matmul docs on 3-D arrays)
    sigma_f = jacobian @ sigma @ jacobian_t

    var_c = sigma_f[:, 0, 0]  # extracts new covariance matrix values
    var_d = sigma_f[:, 1, 1]
    cov_c_d = sigma_f[:, 0, 1]

    return var_c, var_d, cov_c_d
