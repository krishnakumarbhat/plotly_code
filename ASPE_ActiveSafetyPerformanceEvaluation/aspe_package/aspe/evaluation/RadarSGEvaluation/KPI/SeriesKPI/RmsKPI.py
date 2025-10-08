import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.ISeriesKPI import ISeriesKPI


class RmsKPI(ISeriesKPI):
    def __init__(self):
        super().__init__()
        self.kpi_signature = 'rms'

    def calculate(self, series: pd.Series) -> np.float32:
        """
        Method for root mean square calculation.

        :param series: column of numeric values
        :type series: pd.Series

        :rtype: np.float32
        """

        self.validate(series)
        return np.float32(np.sqrt(np.mean(np.square(series))))
