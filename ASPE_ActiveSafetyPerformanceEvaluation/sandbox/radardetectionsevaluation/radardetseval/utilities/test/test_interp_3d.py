
import numpy as np
from pytest import fixture

from radardetseval.utilities.interp_3d import interp_3d


@fixture
def data():
    return np.array([[[1.0, 2.0], [3.0, 4.0]],
                     [[5.0, 8.0], [1.0, 4.0]]])


@fixture
def epsilon():
    return 1.0e-6


class TestInterp3d:
    """

    """
    def test_for_single_x(self, data, epsilon):

        xp = np.array([0.0, 10.0])
        x = np.array([5])

        expected_out = np.array([[[3.0, 5.0], [2.0, 4.0]]])

        observed_out = interp_3d(x, xp, data)

        assert np.allclose(observed_out, expected_out, epsilon)

    def test_for_single_couple_x(self, data, epsilon):

        xp = np.array([0.0, 4.0])
        x = np.array([1, 2, 3])

        expected_out = np.array([[[2.0, 3.5], [2.5, 4.0]],
                                 [[3.0, 5.0], [2.0, 4.0]],
                                 [[4.0, 6.5], [1.5, 4.0]]])

        observed_out = interp_3d(x, xp, data)

        assert np.allclose(observed_out, expected_out, epsilon)



