import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.KPI.ISeriesKPI import ISeriesKPI


class MeanKPI(ISeriesKPI):
    kpi_signature = 'mean'

    def calculate_kpi(self, series):
        return np.mean(series)
