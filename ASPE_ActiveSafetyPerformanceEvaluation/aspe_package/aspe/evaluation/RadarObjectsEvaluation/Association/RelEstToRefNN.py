from typing import Optional, Tuple

import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Association import IAssociation
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import CartesianDistance2D, IPairsFeature


class RelEstToRefNN(IAssociation):
    def __init__(self, assoc_distance_threshold: float = 3.0, distance_function: Optional[IPairsFeature] = None):
        """
        Associate objects within same time instance which are closest to each other. Additional condition is that
        estimated objects relevancy flag must be set to True. Distance metric is defined by
        distance_function parameter - by default cartesian 2D metrics is used which compares centers of objects.
        :param assoc_distance_threshold: distance between objects must be below that value to associate them
        :param distance_function: class which is responsible for calculating distance between objects
        """
        super().__init__()
        self.assoc_distance_threshold = assoc_distance_threshold
        if distance_function is None:
            distance_function = CartesianDistance2D()
        self.distance_function = distance_function

    def associate(self, pe_pairs: pd.DataFrame, pairs_est: pd.DataFrame, pairs_ref: pd.DataFrame) \
            -> Tuple[pd.Series, Optional[pd.DataFrame]]:
        is_associated_series = pd.Series(False, index=pe_pairs.index)
        distance = None

        def min_idx_for_rel_est_dist(df, distance_column_name):
            est_rel_mask = df['relevancy_flag_est']
            filtered_df = df[est_rel_mask]
            if not filtered_df.empty:
                return filtered_df[distance_column_name].idxmin()

        if not pe_pairs.empty:
            pe_pairs_local = pe_pairs.loc[:, ['scan_index', 'unique_id_ref', 'relevancy_flag_est']]
            distance = self.distance_function.calculate(pairs_est, pairs_ref)
            pe_pairs_local = pd.concat([pe_pairs_local, distance], axis=1)
            distance_col_name = distance.columns[0]

            columns_to_group = ['scan_index', 'unique_id_ref']
            pairs_grouped = pe_pairs_local.groupby(by=columns_to_group)
            min_distances_multi_index_array = pairs_grouped.apply(min_idx_for_rel_est_dist,
                                                                  distance_column_name=distance_col_name)
            min_distances_multi_index_array = min_distances_multi_index_array.dropna().values

            if len(min_distances_multi_index_array) > 0:  # if there are any associated pairs
                min_distances_multi_index = pd.Index(min_distances_multi_index_array)
                is_associated_series[min_distances_multi_index] = True
                over_thr_mask = pe_pairs_local.loc[:, distance_col_name] > self.assoc_distance_threshold
                is_associated_series.loc[over_thr_mask] = False
        return is_associated_series, distance


if __name__ == "__main__":
    test_df = pd.DataFrame({
        'scan_index': [0, 0, 1, 1],
        'unique_id_ref': [0, 0, 0, 0],
        'unique_id_est': [1, 2, 1, 2],
        'association_distance': [0.2, 1, 0.2, 1],
        'relevancy_flag_est': [1, 1, 1, 1],
    })
    test_df.drop([0], inplace=True)
    association = RelEstToRefNN(0.8)
    association.associate(test_df)
