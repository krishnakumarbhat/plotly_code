# coding=utf-8
"""
Tests for uncertainty propagation supporting functions
"""
import numpy as np

from aspe.utilities.MathFunctions import calculate_rotation_matrix_values
from aspe.utilities.UncertaintyPropagation import propagate_uncertainty_2d


def test_covariance_matrix_rotated_in_the_right_direction():
    var_A = np.array([4])
    var_B = np.array([3])
    cov_A_B = np.array([0])
    vcs_orientation_in_wcs = np.deg2rad(np.array([30]))

    j_11, j_12, j_21, j_22 = calculate_rotation_matrix_values(-vcs_orientation_in_wcs)
    _, _, cov_C_D = propagate_uncertainty_2d(var_A, var_B, cov_A_B, j_11, j_12, j_21, j_22)

    assert cov_C_D[0] < 0
