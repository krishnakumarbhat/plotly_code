from abc import ABC, abstractmethod

from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class TimeSynchInterface(ABC):

    def __init__(self, *args, **kwargs):
        """
        Init function realise class configuration: all filter conf should be set here

        :param args: any arguments needed for configuration
        :param kwargs: same as above
        """
        pass

    @abstractmethod
    def synch(self,
              master_extracted_data: ExtractedData,
              slave_extracted_data: ExtractedData,
              *args, **kwargs) -> (ExtractedData, ExtractedData):
        """
        :param master_extracted_data: extracted estimation object
        :type master_extracted_data: ExtractedData
        :param slave_extracted_data: extracted reference object
        :type slave_extracted_data: ExtractedData
        :param args:
        :param kwargs:
        :return: (ExtractedData, ExtractedData) -> synchronized_master_extracted_data, synchronized_slave_extracted_data
        """
        raise NotImplementedError(
            'This method is an abstract -> please overwrite it or use or use one of the existing classes')
