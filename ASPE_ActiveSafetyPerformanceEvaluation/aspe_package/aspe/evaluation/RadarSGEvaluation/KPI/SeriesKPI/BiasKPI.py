import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.ISeriesKPI import ISeriesKPI


class BiasKPI(ISeriesKPI):
    def __init__(self):
        super().__init__()
        self.kpi_signature = 'bias'

    def calculate(self, series: pd.Series) -> np.float32:
        """
        Method for bias calculation.

        :param series: column of numeric values
        :type series: pd.Series

        :rtype: np.float32
        """
        self.validate(series)
        return np.float32(np.mean(series))
