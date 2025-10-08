import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.KPI.ISeriesKPI import ISeriesKPI


class RMSEKPI(ISeriesKPI):
    kpi_signature = 'RMSE'

    def calculate_kpi(self, series):
        return np.sqrt(np.mean(np.square(series)))
