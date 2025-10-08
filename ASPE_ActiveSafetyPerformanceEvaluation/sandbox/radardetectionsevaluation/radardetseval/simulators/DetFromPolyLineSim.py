from copy import deepcopy

import numpy as np
from scipy import rand

from radardetseval.Iface.PolyLineDS import PolyLineDS
from scipy.stats import multivariate_normal, uniform

from radardetseval.simulators.sim_det_from_point import sim_single_det_from_scs_cart_point


class DetFromPolyLineSim:
    """
    Simulator of polyline
    """
    def __init__(self, ref_line: PolyLineDS):
        """

        :param ref_line:
        """
        self.ref_line = ref_line

    def sim_single_point_from_vertex(self, meas_cov_matrix: np.array, vertex_id=0.0, unique_id=0.0):
        """

        :param meas_cov_matrix:
        :param vertex_id:
        :param unique_id:
        :return:
        """

        polygon = self.ref_line.signals[self.ref_line.signals.unique_id == unique_id]
        vertex_ids = polygon['vertex_id'].values
        single_point_df = self.ref_line.interpolate_polygon(np.array([vertex_id]), vertex_ids, polygon)
        point_series = single_point_df.iloc[0, :]
        ref_state = np.array([point_series.x, point_series.y, point_series.vx,  point_series.vy])

        detection = sim_single_det_from_scs_cart_point(ref_state, meas_cov_matrix)
        return detection

    def sim_single_point_randomly(self, meas_cov_matrix: np.array):
        """

        :param meas_cov_matrix:
        :param vertex_id:
        :param unique_id:
        :return:
        """

        polygon = self.ref_line.signals
        vertex_ids = polygon['vertex_id'].values
        min_vertex = vertex_ids.min()
        max_vertex = vertex_ids.max()
        vertex_id_draw = uniform.rvs(loc=min_vertex, scale=max_vertex-min_vertex)
        single_point_df = self.ref_line.interpolate_polygon(np.array([vertex_id_draw]), vertex_ids, polygon)
        point_series = single_point_df.iloc[0, :]
        ref_state = np.array([point_series.x, point_series.y, point_series.vx,  point_series.vy])

        detection = sim_single_det_from_scs_cart_point(ref_state, meas_cov_matrix)
        return detection, vertex_id_draw
