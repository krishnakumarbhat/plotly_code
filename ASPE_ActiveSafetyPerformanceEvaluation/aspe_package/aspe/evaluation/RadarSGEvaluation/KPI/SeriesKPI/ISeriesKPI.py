from abc import ABC, abstractmethod

import pandas as pd


class ISeriesKPI(ABC):
    """
    Base abstract class for all KPI's metrics calculation.
    """

    def __init__(self):
        self.kpi_signature = None

    @abstractmethod
    def calculate(self, series: pd.Series):
        pass

    @staticmethod
    def validate(series: pd.Series):
        if series.empty:
            raise ValueError('No data in series')
