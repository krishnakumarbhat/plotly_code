from abc import ABC, abstractmethod
from typing import Tuple

import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEPairedObjects import PEPairedObjects
from aspe.extractors.Interfaces.IObjects import IObjects


class IPEPairedObjectsPreBuilder(ABC):
    def __init__(self):
        self.paired_objects = None
        self.pairs_est_signals = None
        self.pairs_ref_signals = None

    @abstractmethod
    def build(self,
              estimated_data: IObjects,
              reference_data: IObjects) -> Tuple[PEPairedObjects, pd.DataFrame, pd.DataFrame]:
        """

        :param estimated_data:
        :param reference_data:
        :return:
        """
        pass

    def _set_pair_unique_id(self):
        pair_unique_id = 0
        for (id_est, id_ref), unique_pair in self.paired_objects.signals.groupby(by=['unique_id_est', 'unique_id_ref']):
            self.paired_objects.signals.loc[unique_pair.index, 'unique_id'] = pair_unique_id
            pair_unique_id += 1
