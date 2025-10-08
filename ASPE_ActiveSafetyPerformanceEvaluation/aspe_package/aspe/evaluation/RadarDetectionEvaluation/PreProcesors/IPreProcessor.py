from abc import ABC, abstractmethod

from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class IPreProcessor(ABC):

    @abstractmethod
    def pre_process_data(self, estimated_data: ExtractedData, reference_data: ExtractedData, sensor_id: int)  \
            -> (ExtractedData, ExtractedData):
        """
        Pre-process data:

        :param estimated_data:
        :type estimated_data: ExtractedData
        :param reference_data:
        :type reference_data: ExtractedData
        :param sensor_id: sensor which should be filtered
        :type sensor_id: int
        :return: (ExtractedData, ExtractedData)
        """

        raise NotImplementedError('This method is an abstract ->'
                                  ' please overwrite it or use one of the existing classes')
