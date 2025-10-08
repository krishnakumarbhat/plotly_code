import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.FalseNegativeKPI import FalseNegativeKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.ISeriesKPI import ISeriesKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.TruePositiveKPI import TruePositiveKPI


class TPRCalculationKPI(ISeriesKPI):
    def __init__(self):
        super().__init__()
        self.kpi_signature = 'TPR'

    def calculate(self, series: pd.Series) -> np.float32:
        """
        Method for counting true positive ratio.

        :param series: column of classification values
        :type series: pd.Series(BCType)

        :rtype: np.float32
        """

        self.validate(series)
        tp_count = TruePositiveKPI().calculate(series)
        fn_count = FalseNegativeKPI().calculate(series)
        return np.float32(tp_count / (tp_count + fn_count))
