from abc import ABC, abstractmethod


class ISeriesKPI(ABC):
    kpi_signature = None

    @abstractmethod
    def calculate_kpi(self, series):
        pass
