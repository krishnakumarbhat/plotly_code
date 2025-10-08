import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.ISeriesKPI import ISeriesKPI


class TruePositiveKPI(ISeriesKPI):
    def __init__(self):
        super().__init__()
        self.kpi_signature = 'TPs'

    def calculate(self, series: pd.Series(BCType)) -> np.int64:
        """
        Method for counting True Positives.

        Method is expecting pd.Series with BCType values as input,
        otherwise exception is raised.

        :param series: column of classification values
        :type series: pd.Series(BCType)

        :rtype: np.int64
        """

        self.validate(series)
        series_type = series.map(type)
        if np.any(series_type != BCType):
            raise ValueError('binary_classification columns with BCType was expected. ')
        return np.int64(np.sum(BCType.TruePositive == series))
