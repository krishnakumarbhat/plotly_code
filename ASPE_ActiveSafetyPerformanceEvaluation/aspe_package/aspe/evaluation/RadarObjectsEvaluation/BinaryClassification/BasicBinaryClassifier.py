from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.IBinaryClassifier import BCType, IBinaryClassifier
from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEPairedObjects import PEPairedObjects
from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PESingleObjects import PESingleObjects


class BasicBinaryClassifier(IBinaryClassifier):

    def classify_pairs(self, paired_data: PEPairedObjects, *args, **kwargs):
        is_associated_mask = paired_data.signals.loc[:, 'is_associated']
        relevancy_flag_ref = paired_data.signals.loc[:, 'relevancy_flag_ref']
        relevancy_flag_est = paired_data.signals.loc[:, 'relevancy_flag_est']

        true_positive_mask = is_associated_mask & relevancy_flag_ref & relevancy_flag_est
        paired_data.signals.loc[true_positive_mask, 'binary_classification'] = BCType.TruePositive

    def classify_estimated_data(self, est_data: PESingleObjects, *args, **kwargs):
        is_associated_mask = est_data.signals.loc[:, 'is_associated'].values
        relevancy_flag_mask = est_data.signals.loc[:, 'relevancy_flag']

        true_positive_mask = is_associated_mask & relevancy_flag_mask
        false_positive_mask = ~is_associated_mask & relevancy_flag_mask

        est_data.signals.loc[true_positive_mask, 'binary_classification'] = BCType.TruePositive
        est_data.signals.loc[false_positive_mask, 'binary_classification'] = BCType.FalsePositive

    def classify_reference_data(self, ref_data: PESingleObjects, *args, **kwargs):
        is_associated_mask = ref_data.signals.loc[:, 'is_associated'].values
        relevancy_flag_mask = ref_data.signals.loc[:, 'relevancy_flag']

        true_positive_mask = is_associated_mask
        false_negative_mask = ~is_associated_mask & relevancy_flag_mask

        ref_data.signals.loc[true_positive_mask, 'binary_classification'] = BCType.TruePositive
        ref_data.signals.loc[false_negative_mask, 'binary_classification'] = BCType.FalseNegative
