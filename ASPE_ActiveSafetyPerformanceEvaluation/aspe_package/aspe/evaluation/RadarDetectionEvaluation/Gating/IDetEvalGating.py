from abc import ABC, abstractmethod

import pandas as pd

from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class IDetEvalGating(ABC):

    @abstractmethod
    def gating(self, estimated_data: ExtractedData, reference_data: ExtractedData) \
            -> pd.DataFrame:
        """
        Perform gating object vs detections and prepare DataFrames for further processing

        :param estimated_data:
        :type estimated_data: ExtractedData
        :param reference_data:
        :type reference_data: ExtractedData
        :return: pd.DataFrame, valid paris DataFrame with linkage to estimated detections and reference objects
                 valid_pairs['scan_index']
                 valid_pairs['unique_id_obj']
                 valid_pairs['unique_id_det']
                 valid_pairs['gating_distance']
        """

        raise NotImplementedError('This method is an abstract ->'
                                  ' please overwrite it or use one of the existing classes')
