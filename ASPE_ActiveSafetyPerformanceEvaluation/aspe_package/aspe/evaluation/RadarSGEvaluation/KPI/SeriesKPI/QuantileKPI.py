import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.ISeriesKPI import ISeriesKPI


class QuantileKPI(ISeriesKPI):
    """
    Class for quantiles calculation for a series of absolute
    values of given data.

    quantile_value suppose to belong to 0-100 interval,
    otherwise exception is raised.

    :param quantile_value: value from (0, 100) interval
    :type quantile_value: float
    """

    def __init__(self, quantile_value: float):
        super().__init__()
        self.quantile_value = quantile_value
        self.kpi_signature = f'q{quantile_value}'
        self._min_value = 0.0
        self._max_value = 100.0
        self._is_quantile_proper()

    def calculate(self, series: pd.Series) -> np.float32:
        """
        Method for quantile calculation or a series
        of absolute values of given data.

        :param series: column of numeric values
        :type series: pd.Series

        :rtype: np.float32
        """
        self.validate(series)
        series.dropna(inplace=True)
        return np.float32(np.quantile(np.abs(series), 0.01 * self.quantile_value))

    def _is_quantile_proper(self):
        if (self.quantile_value <= self._min_value) or (self.quantile_value >= self._max_value):
            raise ValueError('Expected value from 0:100 interval. ')


class Quantile25KPI(QuantileKPI):
    def __init__(self):
        super().__init__(25)


class Quantile50KPI(QuantileKPI):
    def __init__(self):
        super().__init__(50)


class Quantile75KPI(QuantileKPI):
    def __init__(self):
        super().__init__(75)


class Quantile99KPI(QuantileKPI):
    def __init__(self):
        super().__init__(99)
