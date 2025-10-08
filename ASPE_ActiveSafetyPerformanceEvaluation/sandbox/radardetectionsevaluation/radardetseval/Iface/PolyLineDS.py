import pandas as pd
import numpy as np

from radardetseval.utilities.interp_3d import interp_3d


class PolyLineDS:
    """
    Definition of poly-line Data Set which can be then used in simulators
    """
    state_num = 4
    base_signals_names = [
        'unique_id',  # Unique ID of polygon - several polygons can exists inside one object
        'vertex_id',  # Unique ID of polygon - several polygons can exists inside one object
        'x',
        'y',
        'vx',
        'vy',
        'cov_matrix',
    ]
    signals_names_3d = ['cov_matrix']

    def __init__(self):
        """

        """

        self.signals = pd.DataFrame(columns=self.base_signals_names, dtype='float')
        self.coordinate_system = 'SCS'
        self._current_polyline_id = float(0)
        self._current_vertex_id = float(0)

    def add_point(self,
                  x: float, y: float,
                  vx: float, vy: float,
                  cov: np.array = None):
        """

        :param x:
        :param y:
        :param vx:
        :param vy:
        :param cov:
        :return:
        """

        if cov is None:
            cov = np.diag(np.zeros(self.state_num))

        row = {'unique_id': self._current_polyline_id,
               'vertex_id': self._current_vertex_id,
               'x': x,
               'y': y,
               'vx': vx,
               'vy': vy,
               'cov_matrix': cov}
        self.signals = self.signals.append(row, ignore_index=True)
        self._current_vertex_id += 1

    def end_polygon(self):
        """

        :return:
        """
        self._current_polyline_id += 1
        self._current_vertex_id = 0

    def add_polyline_with_constant_cov(self,
                                       x: np.array, y: np.array,
                                       vx: np.array, vy: np.array,
                                       cov: np.array = None):
        """

        :param x:
        :param y:
        :param vx:
        :param vy:
        :param cov:
        :return:
        """
        if cov is None:
            cov = np.diag(np.zeros(self.state_num))

        self.end_polygon()
        for i, _ in enumerate(x):
            self.add_point(x[i], y[i], vx[i], vy[i], cov)
        self.end_polygon()

    def discretize_single_polygon(self, polygon, dl):
        length_array = self.calc_segment_lengths_in_polygon(polygon)
        length_sum = length_array.sum()
        length_cum_sum = np.concatenate([np.array([0]), length_array.cumsum()])

        n_samples = int(round(length_sum / dl))+1
        length_discrete = np.linspace(0, length_sum, n_samples)
        vertex_ids = polygon['vertex_id'].values
        vertex_discrete = np.interp(length_discrete, length_cum_sum, vertex_ids)

        return self.interpolate_polygon(vertex_discrete, vertex_ids, polygon)

    def interpolate_polygon(self, x, xp, polygon):
        discrete_signals = pd.DataFrame(columns=self.base_signals_names, dtype='float')
        for signal in self.base_signals_names:
            values = polygon[signal].values
            if signal in self.signals_names_3d:
                discrete_signals[signal] = list(interp_3d(x, xp, np.stack(values)))
            else:
                discrete_signals[signal] = np.interp(x, xp, values)
        return discrete_signals

    @staticmethod
    def calc_segment_lengths_in_polygon(polygon):
        length_array = list()

        for i in range(len(polygon)-1):
            length_array.append(np.hypot(polygon['x'].values[i] - polygon['x'].values[i+1],
                                         polygon['y'].values[i] - polygon['y'].values[i+1]))

        return np.array(length_array)