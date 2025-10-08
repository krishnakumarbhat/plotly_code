import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.IBinaryClassifier import BCType
from aspe.evaluation.RadarObjectsEvaluation.DataStructures import PESingleObjects
from aspe.evaluation.RadarObjectsEvaluation.KPI.IBinaryClassificationKPI import IBinaryClassificationKPI


class F1ScoreBinaryClassificationKPI(IBinaryClassificationKPI):
    def __init__(self, *args, **kwargs):
        super().__init__('F1_score')

    def calculate_kpi(self, reference_objects: PESingleObjects, estimated_objects: PESingleObjects):
        est_binary_classification = estimated_objects.signals.loc[:, 'binary_classification'].values
        ref_binary_classification = reference_objects.signals.loc[:, 'binary_classification'].values
        tp_count = np.sum(est_binary_classification == BCType.TruePositive)
        fn_count = np.sum(ref_binary_classification == BCType.FalseNegative)
        fp_count = np.sum(est_binary_classification == BCType.FalsePositive)
        tpr = tp_count / (tp_count + fn_count)
        ppv = tp_count / (tp_count + fp_count)
        f1_score = 2 * (tpr * ppv) / (tpr + ppv)
        return f1_score