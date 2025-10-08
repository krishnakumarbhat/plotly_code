from typing import Optional, Tuple

import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Association import IAssociation
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import CartesianDistance2D
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.intersection_over_union_ratio import (
    IntersectionOverUnionRatio,
)


class OneToManyAssociation(IAssociation):
    def __init__(self, bbox_intersection_condition=False):
        """
        """
        super().__init__()
        self.distance_function = CartesianDistance2D()
        self.bbox_intersection_condition = bbox_intersection_condition

    def associate(self, pe_pairs: pd.DataFrame, pairs_est: pd.DataFrame, pairs_ref: pd.DataFrame) \
            -> Tuple[pd.Series, Optional[pd.DataFrame]]:
        is_associated_series = pd.Series(False, index=pe_pairs.index)
        distance = None
        additional_info = None

        if self.bbox_intersection_condition:
            if "intersection_over_union" not in pe_pairs:
                intersection_over_union_df = IntersectionOverUnionRatio().calculate(pairs_est, pairs_ref)
                pe_pairs = pd.concat([pe_pairs, intersection_over_union_df], axis=1)

            pe_pairs = pe_pairs[pe_pairs["intersection_over_union"] > 0]

        def min_idx_for_rel_est_dist(df, distance_column_name):
            return df[distance_column_name].idxmin()

        if not pe_pairs.empty:
            pe_pairs_local = pe_pairs.loc[:, ['scan_index', 'unique_id_ref', 'unique_id_est', 'relevancy_flag_est']]
            distance = self.distance_function.calculate(pairs_est, pairs_ref)
            pe_pairs_local = pd.concat([pe_pairs_local, distance], axis=1)
            distance_col_name = distance.columns[0]

            columns_to_group = ['scan_index', 'unique_id_ref']
            pairs_grouped = pe_pairs_local.groupby(by=columns_to_group)
            min_distances_multi_index_array = pairs_grouped.apply(min_idx_for_rel_est_dist,
                                                                  distance_column_name=distance_col_name)
            min_distances_multi_index_array = min_distances_multi_index_array.dropna().values

            relevant_pairs = pe_pairs[pe_pairs.relevancy_flag_est | pe_pairs.relevancy_flag_ref]
            is_associated_series[relevant_pairs.index] = True

            best_match = pe_pairs_local['unique_id_est'][min_distances_multi_index_array]

            additional_info = pd.concat([best_match, distance], axis=1)
            additional_info.columns = ['best_est_match', 'distance']

        return is_associated_series, additional_info


if __name__ == "__main__":
    test_df = pd.DataFrame({
        'scan_index': [0, 0, 1, 1],
        'unique_id_ref': [0, 0, 0, 0],
        'unique_id_est': [1, 2, 1, 2],
        'association_distance': [0.2, 1, 0.2, 1],
    })
    test_df.drop([0], inplace=True)
    association = OneToManyAssociation(0.8)
    association.associate(test_df)
