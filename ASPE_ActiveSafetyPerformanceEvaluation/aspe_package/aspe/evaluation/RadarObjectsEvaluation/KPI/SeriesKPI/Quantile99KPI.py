import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.KPI.ISeriesKPI import ISeriesKPI


class Quantile99KPI(ISeriesKPI):
    kpi_signature = 'Q99'

    def calculate_kpi(self, series):
        if len(series) > 0:
            kpi_val = np.quantile(np.abs(series), 0.99)
        else:
            kpi_val = np.nan
        return kpi_val
