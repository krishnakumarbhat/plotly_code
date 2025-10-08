import numpy as np
import pandas as pd
import pytest

from aspe.extractors.Transform.cs_transform_cov import rotate_2d_cov


@pytest.fixture()
def atol():
    return 1.0e-6


class TestRotate2DCov:

    def test_0_degree_rot(self, atol):
        var_a = np.array([1.0])
        var_b = np.array([4.0])
        cov_ab = np.array([0.5])

        rotation = np.deg2rad(0.0)

        exp_var_c = var_a
        exp_var_d = var_b
        exp_cov_ab = cov_ab

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(cov_cd, exp_cov_ab, atol=atol)

    def test_45_degree_rot_no_cov(self, atol):
        var_a = np.array([1.0])
        var_b = np.array([4.0])
        cov_ab = np.array([0.0])

        rotation = np.deg2rad(45.0)

        exp_var_c = np.array([2.5])
        exp_var_d = np.array([2.5])
        exp_cov_ab = np.array([-1.5])

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(cov_cd, exp_cov_ab, atol=atol)

    def test_45_degree_rot_cov(self, atol):
        var_a = np.array([1.0])
        var_b = np.array([1.0])
        cov_ab = np.array([1.0])

        rotation = np.deg2rad(45.0)

        exp_var_c = np.array([0.0])
        exp_var_d = np.array([2.0])
        exp_cov_ab = np.array([0.0])

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(cov_cd, exp_cov_ab, atol=atol)

    def test_minus_45_degree_rot_cov(self, atol):
        var_a = np.array([1.0])
        var_b = np.array([1.0])
        cov_ab = np.array([1.0])

        rotation = np.deg2rad(-45.0)

        exp_var_c = np.array([2.0])
        exp_var_d = np.array([0.0])
        exp_cov_ab = np.array([0.0])

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(cov_cd, exp_cov_ab, atol=atol)

    def test_90_degree_rot(self, atol):
        var_a = pd.array([1.0])
        var_b = np.array([4.0])
        cov_ab = np.array([0.5])

        rotation = np.deg2rad(90.0)

        exp_var_c = var_b
        exp_var_d = var_a
        exp_cov_ab = -cov_ab

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(cov_cd, exp_cov_ab, atol=atol)

    def test_minus_90_degree_rot(self, atol):
        var_a = np.array([1.0])
        var_b = np.array([4.0])
        cov_ab = np.array([0.5])

        rotation = np.deg2rad(-90.0)

        exp_var_c = var_b
        exp_var_d = var_a
        exp_cov_ab = -cov_ab

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(exp_cov_ab, exp_cov_ab, atol=atol)

    def test_180_degree_rot(self, atol):
        var_a = np.array([1.0])
        var_b = np.array([4.0])
        cov_ab = np.array([0.5])

        rotation = np.deg2rad(180.0)

        exp_var_c = var_a
        exp_var_d = var_b
        exp_cov_ab = cov_ab

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(cov_cd, exp_cov_ab, atol=atol)

    def test_for_single_value_series(self, atol):
        var_a = pd.Series([1.0])
        var_b = pd.Series([4.0])
        cov_ab = pd.Series([0.5])

        rotation = np.deg2rad(0.0)

        exp_var_c = var_a
        exp_var_d = var_b
        exp_cov_ab = cov_ab

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(cov_cd, exp_cov_ab, atol=atol)

    def test_for_series(self, atol):
        var_a = pd.Series([1.0, 4.0])
        var_b = pd.Series([4.0, 8.0])
        cov_ab = pd.Series([0.5, 2.0])

        rotation = np.deg2rad(90.0)

        exp_var_c = var_b.values
        exp_var_d = var_a.values
        exp_cov_ab = -cov_ab.values

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(cov_cd, exp_cov_ab, atol=atol)

    def test_rotation_array(self, atol):
        var_a = np.array([1.0, 1.0])
        var_b = np.array([1.0, 1.0])
        cov_ab = np.array([1.0, 1.0])

        rotation = np.deg2rad([45.0, -45])

        exp_var_c = np.array([0.0, 2.0])
        exp_var_d = np.array([2.0, 0.0])
        exp_cov_ab = np.array([0.0, 0.0])

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(cov_cd, exp_cov_ab, atol=atol)

    def test_rotation_single_array(self, atol):
        var_a = np.array([1.0, 1.0])
        var_b = np.array([1.0, 1.0])
        cov_ab = np.array([1.0, 1.0])

        rotation = np.deg2rad([45.0])

        exp_var_c = np.array([0.0, 0.0])
        exp_var_d = np.array([2.0, 2.0])
        exp_cov_ab = np.array([0.0, 0.0])

        var_c, var_d, cov_cd = rotate_2d_cov(var_a, var_b, cov_ab, rotation)

        np.testing.assert_allclose(var_c, exp_var_c, atol=atol)
        np.testing.assert_allclose(var_d, exp_var_d, atol=atol)
        np.testing.assert_allclose(cov_cd, exp_cov_ab, atol=atol)

    def test_int_input_neg(self, atol):
        var_a = np.array([1.0])
        var_b = np.array([4.0])
        cov_ab = np.array([0.5])

        rotation = 2

        with pytest.raises(ValueError):
            rotate_2d_cov(var_a, var_b, cov_ab, rotation)
