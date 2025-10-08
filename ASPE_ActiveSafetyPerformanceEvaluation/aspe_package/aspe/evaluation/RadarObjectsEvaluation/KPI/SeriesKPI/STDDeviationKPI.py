import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.KPI.ISeriesKPI import ISeriesKPI


class STDKPI(ISeriesKPI):
    kpi_signature = 'STD'

    def calculate_kpi(self, series):
        return np.std(series)
