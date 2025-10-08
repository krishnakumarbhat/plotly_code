import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.KPI.ISeriesKPI import ISeriesKPI


class MinValueKPI(ISeriesKPI):
    kpi_signature = 'min'

    def calculate_kpi(self, series):
        if len(series) > 0:
            kpi_val = np.min(np.abs(series))
        else:
            kpi_val = np.nan
        return kpi_val
