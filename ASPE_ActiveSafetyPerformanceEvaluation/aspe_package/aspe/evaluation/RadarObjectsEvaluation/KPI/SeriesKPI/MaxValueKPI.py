import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.KPI.ISeriesKPI import ISeriesKPI


class MaxValueKPI(ISeriesKPI):
    kpi_signature = 'max'

    def calculate_kpi(self, series):
        if len(series) > 0:
            kpi_val = np.max(np.abs(series))
        else:
            kpi_val = np.nan
        return kpi_val
