from abc import ABC, abstractmethod

from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Interfaces.IDataSet import IDataSet


class ScanIndexSynchInterface(ABC):

    def __init__(self, *args, **kwargs):
        """
        Init function realise class configuration: all filter conf should be set here

        :param args: any arguments needed for configuration
        :param kwargs: same as above
        """
        pass

    @abstractmethod
    def synch(self,
              estimated_data: ExtractedData,
              reference_data: ExtractedData,
              master_data_set: IDataSet,
              slave_data_set: IDataSet,
              *args, **kwargs) -> (ExtractedData, ExtractedData):
        """
        :param estimated_data: extracted estimation object
        :type estimated_data: ExtractedData
        :param reference_data: extracted reference object
        :type reference_data: ExtractedData
        :param master_data_set: master DataSet which is a base for synchronization
        :type master_data_set: IDataSet
        :param slave_data_set: slave DataSet which is a base for synchronization
        :type slave_data_set: IDataSet
        :param args:
        :param kwargs:
        :return: (ExtractedData, ExtractedData) -> synchronized_reference_data, synchronized_estimated_data
        """
        raise NotImplementedError(
            'This method is an abstract -> please overwrite it or use or use one of the existing classes')
