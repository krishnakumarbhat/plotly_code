import typing

import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEPairedObjects import PEPairedObjects
from aspe.evaluation.RadarObjectsEvaluation.PairsPreBuilders import IPEPairedObjectsPreBuilder
from aspe.extractors.Interfaces.IObjects import IObjects


class PEPairedObjectsPreBuilderAll(IPEPairedObjectsPreBuilder):
    def __init__(self):
        super().__init__()
        self._pre_paired = None

    def build(self,
              estimated_data: IObjects,
              reference_data: IObjects) -> typing.Tuple[PEPairedObjects, pd.DataFrame, pd.DataFrame]:
        """

        :param estimated_data:
        :param reference_data:
        :return:
        """
        self.pairs_est_signals = estimated_data.signals.copy()
        self.pairs_ref_signals = reference_data.signals.copy()
        self.pairs_est_signals['index_est'] = self.pairs_est_signals.index
        self.pairs_ref_signals['index_ref'] = self.pairs_ref_signals.index

        self._pair_all_objects_within_same_scan()

        self._pre_paired.rename(columns={'timestamp_est': 'timestamp'}, inplace=True)
        self.paired_objects = PEPairedObjects()

        signal_names_logic_sum = list(set(self._pre_paired.columns) &
                                      set(self.paired_objects.signals.columns))
        self.paired_objects.signals[signal_names_logic_sum] = \
            self._pre_paired[signal_names_logic_sum].reset_index(drop=True)
        self._set_pair_unique_id()

        return self.paired_objects, self.pairs_est_signals, self.pairs_ref_signals

    def _pair_all_objects_within_same_scan(self):
        self._pre_paired = self.pairs_ref_signals.join(self.pairs_est_signals.set_index('scan_index'),
                                                       how='inner', on='scan_index', lsuffix='_ref', rsuffix='_est')\
                                                       .reset_index(drop=True)

        ref_paired_indexes = self._pre_paired.loc[:, 'index_ref']
        est_paired_indexes = self._pre_paired.loc[:, 'index_est']
        self.pairs_est_signals = self.pairs_est_signals.loc[est_paired_indexes, :].reset_index(drop=True)
        self.pairs_ref_signals = self.pairs_ref_signals.loc[ref_paired_indexes, :].reset_index(drop=True)