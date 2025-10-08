import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.ISeriesKPI import ISeriesKPI


class StdKPI(ISeriesKPI):
    def __init__(self):
        super().__init__()
        self.kpi_signature = 'std'

    def calculate(self, series: pd.Series) -> np.float32:
        """
        Method for standard deviation calculation.

        :param series: column of numeric values
        :type series: pd.Series

        :rtype: np.float32
        """

        self.validate(series)
        return np.float32(np.std(series))
