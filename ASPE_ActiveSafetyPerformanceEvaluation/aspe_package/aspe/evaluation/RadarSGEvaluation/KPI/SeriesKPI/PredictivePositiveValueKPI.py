import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.FalsePositiveKPI import FalsePositiveKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.ISeriesKPI import ISeriesKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.TruePositiveKPI import TruePositiveKPI


class PPVCalculationKPI(ISeriesKPI):
    def __init__(self):
        super().__init__()
        self.kpi_signature = 'PPV'

    def calculate(self, series: pd.Series) -> np.float32:
        """
        Method for counting predictive positive value.

        :param series: column of classification values
        :type series: pd.Series(BCType)

        :rtype: np.float32
        """

        self.validate(series)
        tp_count = TruePositiveKPI().calculate(series)
        fp_count = FalsePositiveKPI().calculate(series)

        return np.float32(tp_count / (tp_count + fp_count))
