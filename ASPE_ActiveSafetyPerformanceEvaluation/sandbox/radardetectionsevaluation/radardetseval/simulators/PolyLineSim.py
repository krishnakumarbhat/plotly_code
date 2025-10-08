from copy import deepcopy

import numpy as np
from radardetseval.Iface.PolyLineDS import PolyLineDS
from scipy.stats import multivariate_normal


class PolyLineSim:
    """
    Simulator of polyline
    """
    def __init__(self, ref_line: PolyLineDS):
        """

        :param ref_line:
        """
        self.ref_line = ref_line

    def sim_with_the_same_deviation(self, cov_matrix: np.array):
        """

        :param cov_matrix:
        :return:
        """

        deviation = multivariate_normal.rvs(mean=np.zeros(self.ref_line.state_num),
                                            cov=cov_matrix,
                                            size=1)
        deviation = deviation.reshape([1, self.ref_line.state_num])
        deviation = deviation.repeat(self.ref_line.signals.shape[0], axis=0)
        return self._fill_output(deviation, cov_matrix)

    def sim_with_the_different_deviations(self, cov_matrix: np.array):
        """

        :param cov_matrix:
        :return:
        """

        deviation = multivariate_normal.rvs(mean=np.zeros(self.ref_line.state_num),
                                            cov=cov_matrix,
                                            size=self.ref_line.signals.shape[0])
        return self._fill_output(deviation, cov_matrix)

    def _fill_output(self, deviation, cov_matrix):
        """

        :param deviation:
        :param cov_matrix:
        :return:
        """
        output = deepcopy(self.ref_line)

        n = deviation.shape[0]
        output.signals['x'] += deviation[:, 0]
        output.signals['y'] += deviation[:, 1]
        output.signals['vx'] += deviation[:, 2]
        output.signals['vy'] += deviation[:, 3]
        output.signals['cov_matrix'] = list(np.repeat(np.array([cov_matrix]), n, axis=0))

        return output




