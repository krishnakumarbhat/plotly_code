import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.ISeriesKPI import ISeriesKPI


class MeanAbsKPI(ISeriesKPI):
    def __init__(self):
        super().__init__()
        self.kpi_signature = 'mean_abs'

    def calculate(self, series: pd.Series) -> np.float32:
        """
        Method for mean absolute value calculation.

        :param series: column of numeric values
        :type series: pd.Series

        :rtype: np.float32
        """

        self.validate(series)
        return np.float32(np.mean(np.abs(series)))
