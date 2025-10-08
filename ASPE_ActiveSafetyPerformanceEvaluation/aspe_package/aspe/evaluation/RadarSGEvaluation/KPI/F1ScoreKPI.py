import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.PredictivePositiveValueKPI import PPVCalculationKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.TruePositiveRatioKPI import TPRCalculationKPI


class F1ScoreKPI:
    def __init__(self):
        self.kpi_signature = 'F1_score'

    @staticmethod
    def calculate(estimated_series: pd.Series(BCType),
                  reference_series: pd.Series(BCType)) -> np.float32:
        """
        Method for calculation of F1_score.

        Method is expecting two pd.Series with BCType values as input,
        otherwise exception is raised.

        :param estimated_series: column from estimated data with BCType values
        :type estimated_series: pd.Series(BCType)
        :param reference_series: column from reference data with BCType values
        :type reference_series: pd.Series(BCType)


        :rtype: np.float32
        """

        if estimated_series.empty | reference_series.empty:
            raise ValueError('No data in at least one of series.')
        estimated_series_type = estimated_series.map(type)
        if np.any(estimated_series_type != BCType):
            raise ValueError('Binary_classification column with BCType was expected for estimated data.')
        reference_series_type = reference_series.map(type)
        if np.any(reference_series_type != BCType):
            raise ValueError('Binary_classification column with BCType was expected for reference data.')

        tpr_value = TPRCalculationKPI().calculate(reference_series)
        ppv_value = PPVCalculationKPI().calculate(estimated_series)
        return np.float32(2 * (tpr_value*ppv_value) / (tpr_value+ppv_value))
