import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.KPI.ISeriesKPI import ISeriesKPI


class BiasKPI(ISeriesKPI):
    kpi_signature = 'bias'

    def calculate_kpi(self, series):
        return np.mean(series)
