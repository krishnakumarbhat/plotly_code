import numpy as np
import pandas as pd

from aspe.evaluation.RadarSGEvaluation.Enums.SegmentationType import SegmentationType
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.ISeriesKPI import ISeriesKPI


class UnderSegmentationRatioKPI(ISeriesKPI):
    def __init__(self):
        super().__init__()
        self.kpi_signature = 'undersegmentation_ratio'

    def calculate(self, series: pd.Series(SegmentationType)) -> np.float32:
        """
        Method for counting under segmentation ratio.

        Method is expecting pd.Series with SegmentationType values as input,
        otherwise exception is raised.

        :param series: column of SegmentationType values
        :type series: pd.Series(SegmentationType)

        :rtype: np.float32
        """

        self.validate(series)
        series_type = series.map(type)
        if np.any(series_type != SegmentationType):
            raise ValueError('multiple_segmentation column with SegmentationType values was expected. ')
        return np.float32(np.sum(SegmentationType.UnderSegmentation == series) / series.count())
