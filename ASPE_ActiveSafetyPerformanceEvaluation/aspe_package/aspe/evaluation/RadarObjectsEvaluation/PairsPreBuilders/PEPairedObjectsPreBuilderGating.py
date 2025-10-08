from typing import Tuple

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEPairedObjects import PEPairedObjects
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.distance_metrics.CartesianDistance2D import (
    CartesianDistance2D,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.distance_metrics.OrientationDifference import (
    OrientationDifference,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsPreBuilders.IPEPairedObjectsPreBuilder import (
    IPEPairedObjectsPreBuilder,
)
from aspe.extractors.Interfaces.IObjects import IObjects


class PEPairedObjectsPreBuilderGating(IPEPairedObjectsPreBuilder):
    def __init__(self,
                 distance_threshold=3.0,
                 velocity_difference_threshold=None,
                 orientation_difference_threshold=None,
                 f_optimize_memory_usage=False,
                 f_add_gating_dist_column=True,
                 bbox_intersection_condition=False):
        super().__init__()

        self.distance_threshold = distance_threshold
        self.velocity_difference_threshold = velocity_difference_threshold
        self.orientation_difference_threshold = orientation_difference_threshold
        self.f_optimize_memory_usage = f_optimize_memory_usage
        self.f_add_gating_dist_column = f_add_gating_dist_column

        self._pre_paired = None

    def build(self, estimated_data: IObjects, reference_data: IObjects) -> Tuple[
        PEPairedObjects, pd.DataFrame, pd.DataFrame]:
        """

        :param estimated_data:
        :param reference_data:
        :return:
        """
        self.pairs_est_signals = estimated_data.signals.copy()
        self.pairs_ref_signals = reference_data.signals.copy()
        self.pairs_est_signals['index_est'] = self.pairs_est_signals.index
        self.pairs_ref_signals['index_ref'] = self.pairs_ref_signals.index

        self._pair_objects()

        self._pre_paired.rename(columns={'timestamp_est': 'timestamp'}, inplace=True)
        self.paired_objects = PEPairedObjects()

        signal_names_logic_sum = list(set(self._pre_paired.columns) &
                                      set(self.paired_objects.signals.columns))
        self.paired_objects.signals[signal_names_logic_sum] = self._pre_paired[signal_names_logic_sum].reset_index(
            drop=True)

        if self.f_add_gating_dist_column:
            self._add_gating_distance_column_to_output()
        self._set_pair_unique_id()
        return self.paired_objects, self.pairs_est_signals, self.pairs_ref_signals

    def _pair_objects(self):
        pairs_est_signals = self.pairs_est_signals
        pairs_ref_signals = self.pairs_ref_signals

        if self.f_optimize_memory_usage:
            pairs_est_signals_min = pairs_est_signals[['scan_index', 'index_est']]
            pairs_ref_signals_min = pairs_ref_signals[['scan_index', 'index_ref']]

            pairs_est_groupby = pairs_est_signals_min.groupby('scan_index')
            pairs_ref_groupby = pairs_ref_signals_min.groupby('scan_index')

            pairs_ref_scan_indices = pairs_ref_signals['scan_index'].drop_duplicates().values
            valid_pairs_list = []

            for si, pairs_est in pairs_est_groupby:
                if si in pairs_ref_scan_indices:
                    pairs_ref = pairs_ref_groupby.get_group(si)

                    all_pairs = pd.merge(pairs_ref, pairs_est, on='scan_index', suffixes=('_ref', '_est'))
                    all_pairs.reset_index(drop=True, inplace=True)

                    ref_paired_indexes = all_pairs.loc[:, 'index_ref']
                    est_paired_indexes = all_pairs.loc[:, 'index_est']

                    distance = self.distance_function.calculate(
                        pairs_ref_signals.loc[ref_paired_indexes],
                        pairs_est_signals.loc[est_paired_indexes])
                    distance.reset_index(drop=True, inplace=True)
                    distance_col_name = distance.columns[0]

                    all_pairs['distance'] = distance[distance_col_name].to_numpy()

                    below_dist_thr_mask = all_pairs['distance'] < self.distance_threshold
                    valid_pairs = all_pairs.loc[below_dist_thr_mask]
                    valid_pairs.reset_index(drop=True, inplace=True)

                    valid_pairs_list.append(valid_pairs)

            pre_paired = pd.concat(valid_pairs_list)

            ref_paired_indexes = pre_paired.loc[:, 'index_ref']
            est_paired_indexes = pre_paired.loc[:, 'index_est']
            pairs_est_signals = pairs_est_signals.loc[est_paired_indexes, :]
            pairs_ref_signals = pairs_ref_signals.loc[ref_paired_indexes, :]
            pairs_est_signals.reset_index(drop=True, inplace=True)
            pairs_ref_signals.reset_index(drop=True, inplace=True)

            pre_paired = pd.concat((
                pairs_est_signals.add_suffix('_est'),
                pairs_ref_signals.add_suffix('_ref'),
                pre_paired['distance'].reset_index(drop=True),
            ), axis=1)
            pre_paired.drop(columns='scan_index_ref', inplace=True)
            pre_paired.rename(columns={
                'scan_index_est': 'scan_index',
                'index_est_est': 'index_est',
                'index_ref_ref': 'index_ref',
            }, inplace=True)

        else:
            pre_paired = pairs_ref_signals.join(pairs_est_signals.set_index('scan_index'),
                                                how='inner', on='scan_index', lsuffix='_ref', rsuffix='_est') \
                .reset_index(drop=True)

            ref_paired_indexes = pre_paired.loc[:, 'index_ref']
            est_paired_indexes = pre_paired.loc[:, 'index_est']
            pairs_est_signals = pairs_est_signals.loc[est_paired_indexes, :].reset_index(drop=True)
            pairs_ref_signals = pairs_ref_signals.loc[ref_paired_indexes, :].reset_index(drop=True)

            valid_objects_mask = np.ones((len(pre_paired),), dtype=bool)

            distance_function = CartesianDistance2D(pos_x_signal_name='center_x', pos_y_signal_name='center_y')
            distance = distance_function.calculate(pairs_ref_signals, pairs_est_signals)
            pre_paired['distance'] = distance[distance.columns[0]].to_numpy()
            valid_objects_mask &= (pre_paired['distance'] < self.distance_threshold)

            if self.velocity_difference_threshold is not None:
                velocity_difference_function = CartesianDistance2D(pos_x_signal_name='velocity_otg_x',
                                                                   pos_y_signal_name='velocity_otg_y')
                velocity_difference = velocity_difference_function.calculate(pairs_ref_signals, pairs_est_signals)
                pre_paired['velocity_difference'] = velocity_difference[velocity_difference.columns[0]].to_numpy()
                valid_objects_mask &= (pre_paired['velocity_difference'] < self.velocity_difference_threshold)

            if self.orientation_difference_threshold is not None:
                orientation_difference_function = OrientationDifference()
                orientation_difference = orientation_difference_function.calculate(pairs_ref_signals, pairs_est_signals)
                orientation_difference_col_name = orientation_difference.columns[0]
                pre_paired['orientation_difference'] = orientation_difference[
                    orientation_difference_col_name].to_numpy()
                valid_objects_mask &= (
                            abs(pre_paired['orientation_difference']) < self.orientation_difference_threshold)


            pre_paired = pre_paired.loc[valid_objects_mask, :].reset_index(drop=True)

            pairs_ref_signals = pairs_ref_signals.loc[valid_objects_mask, :].reset_index(drop=True)
            pairs_est_signals = pairs_est_signals.loc[valid_objects_mask, :].reset_index(drop=True)

        self._pre_paired = pre_paired
        self.pairs_est_signals = pairs_est_signals
        self.pairs_ref_signals = pairs_ref_signals

    def _add_gating_distance_column_to_output(self):
        self.paired_objects.signals['gating_distance'] = self._pre_paired.loc[:, 'distance'].values
