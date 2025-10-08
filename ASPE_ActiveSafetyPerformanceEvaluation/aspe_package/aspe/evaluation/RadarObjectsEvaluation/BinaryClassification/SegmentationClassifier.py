import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.IBinaryClassifier import BCType, IBinaryClassifier
from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEPairedObjects import PEPairedObjects
from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PESingleObjects import PESingleObjects


class SegmentationClassifier(IBinaryClassifier):

    def classify_pairs(self, paired_data: PEPairedObjects, *args, **kwargs):
        is_associated_mask = paired_data.signals.loc[:, 'is_associated']

        true_positive_mask = is_associated_mask
        oversegmentation_mask = np.zeros((len(true_positive_mask)))
        undersegmentation_mask = np.zeros((len(true_positive_mask)))

        if 'best_est_match' in paired_data.signals:
            paired_data.signals['best_est_match'] = paired_data.signals['best_est_match'].fillna(-1)
            paired_data.signals['num_est_matches'] = \
                paired_data.signals.query('is_associated==True').groupby(
                    ['scan_index', 'unique_id_ref'])['unique_id_est'].transform('count')
            paired_data.signals['num_est_matches'] = paired_data.signals['num_est_matches'].fillna(0)

            paired_data.signals['num_ref_matches'] = \
                paired_data.signals.query('is_associated==True').groupby(
                    ['scan_index', 'unique_id_est'])['unique_id_ref'].transform('count')
            paired_data.signals['num_ref_matches'] = paired_data.signals['num_ref_matches'].fillna(0)
        else:
            pass

        if 'num_est_matches' in paired_data.signals:
            only_one_est_match = paired_data.signals.num_est_matches == 1
            oversegmentation_mask = is_associated_mask & ~only_one_est_match
            true_positive_mask &= only_one_est_match

        if 'num_ref_matches' in paired_data.signals:
            only_one_ref_match = paired_data.signals.num_ref_matches == 1
            undersegmentation_mask = is_associated_mask & ~only_one_ref_match
            true_positive_mask &= only_one_ref_match

        paired_data.signals.loc[true_positive_mask, 'binary_classification'] = BCType.TruePositive
        paired_data.signals.loc[oversegmentation_mask, 'binary_classification'] = BCType.OverSegmentation
        paired_data.signals.loc[undersegmentation_mask, 'binary_classification'] = BCType.UnderSegmentation

    def classify_estimated_data(self, est_data: PESingleObjects, *args, **kwargs):
        is_associated_mask = est_data.signals.loc[:, 'is_associated'].values
        relevancy_flag_mask = est_data.signals.loc[:, 'relevancy_flag'].values
        paired_data = args[0]

        true_positive_mask = is_associated_mask & relevancy_flag_mask
        false_positive_mask = ~is_associated_mask & relevancy_flag_mask
        oversegmentation_mask = np.zeros((len(true_positive_mask)))
        undersegmentation_mask = np.zeros((len(true_positive_mask)))

        est_assoc_idxs = paired_data.signals.index_est[paired_data.signals.is_associated]

        est_data.signals.loc[true_positive_mask, 'binary_classification'] = BCType.TruePositive
        est_data.signals.loc[false_positive_mask, 'binary_classification'] = BCType.FalsePositive

        if 'num_ref_matches' in paired_data.signals:
            num_ref = paired_data.signals.loc[est_assoc_idxs.index, 'num_ref_matches']
            index_est = paired_data.signals.loc[num_ref.index, 'index_est']
            num_ref.index = index_est
            est_data.signals.loc[index_est, 'num_ref_matches'] = num_ref
            num_est = paired_data.signals.loc[est_assoc_idxs.index, 'num_est_matches']
            index_est = paired_data.signals.loc[num_est.index, 'index_est']
            num_est.index = index_est
            est_data.signals.loc[index_est, 'num_est_matches'] = num_est
            oversegmentation_mask = est_data.signals.loc[:, 'num_ref_matches'] > 1
            undersegmentation_mask = est_data.signals.loc[:, 'num_est_matches'] > 1

        est_data.signals.loc[oversegmentation_mask, 'binary_classification'] = BCType.UnderSegmentation
        est_data.signals.loc[undersegmentation_mask, 'binary_classification'] = BCType.OverSegmentation

    def classify_reference_data(self, ref_data: PESingleObjects, *args, **kwargs):
        paired_data = args[0]
        bin_class_pairs = paired_data.signals.loc[:, ['index_ref', 'binary_classification']]
        under_segm = bin_class_pairs.loc[bin_class_pairs.binary_classification == BCType.UnderSegmentation, :]
        over_segm = bin_class_pairs.loc[bin_class_pairs.binary_classification == BCType.OverSegmentation, :]

        is_associated_mask = ref_data.signals.loc[:, 'is_associated'].values
        relevancy_flag_mask = ref_data.signals.loc[:, 'relevancy_flag'].values

        true_positive_mask = is_associated_mask
        false_negative_mask = ~is_associated_mask & relevancy_flag_mask

        ref_data.signals.loc[true_positive_mask, 'binary_classification'] = BCType.TruePositive
        ref_data.signals.loc[false_negative_mask, 'binary_classification'] = BCType.FalseNegative
        ref_data.signals.loc[under_segm.index_ref, 'binary_classification'] = BCType.TruePositive.UnderSegmentation
        ref_data.signals.loc[over_segm.index_ref, 'binary_classification'] = BCType.TruePositive.OverSegmentation
