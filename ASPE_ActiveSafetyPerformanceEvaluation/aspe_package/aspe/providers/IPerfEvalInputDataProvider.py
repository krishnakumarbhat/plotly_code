from abc import ABC, abstractmethod
from typing import Tuple

from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class IPerfEvalInputDataProvider(ABC):
    """
    Interface definition of performance evaluation input provider class type.
    """

    @abstractmethod
    def get_single_log_data(self, log_path: str) -> Tuple[ExtractedData, ExtractedData]:
        """
        Get pair of estimated and reference data, which can be used as performance evaluation input. Processing scheme:
        - extract data which needs to be evaluated using log_path
        - find corresponding reference data - this step can be different depending on use case and data types
        - extract reference data
        - perform scan-index and time synchronization between data sets
        - return extracted estimated data and extracted reference data as tuple
        :param log_path: path to evaluated log
        :return: (estimated_data, reference_data)
        """
        pass
