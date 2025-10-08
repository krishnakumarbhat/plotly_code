from abc import ABC, abstractmethod

from aspe.extractors.Interfaces.IDataSet import IDataSet


class IBinaryClassificationKPI(ABC):
    def __init__(self, kpi_signature: str, *args, **kwargs):
        self.kpi_signature = kpi_signature

    @abstractmethod
    def calculate_kpi(self, reference_objects: IDataSet, estimated_objects: IDataSet):
        raise NotImplementedError('Abstract method should be implemented')
