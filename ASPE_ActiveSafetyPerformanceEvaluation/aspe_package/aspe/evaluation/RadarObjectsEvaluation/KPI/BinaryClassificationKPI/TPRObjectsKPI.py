import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.IBinaryClassifier import BCType
from aspe.evaluation.RadarObjectsEvaluation.DataStructures import PESingleObjects
from aspe.evaluation.RadarObjectsEvaluation.KPI.IBinaryClassificationKPI import IBinaryClassificationKPI


class TPRBinaryClassificationKPI(IBinaryClassificationKPI):
    def __init__(self, *args, **kwargs):
        super().__init__('TPR')

    def calculate_kpi(self, reference_objects: PESingleObjects, estimated_objects: PESingleObjects):
        ref_binary_classification = reference_objects.signals.loc[:, 'binary_classification'].values
        tp_count = np.sum(ref_binary_classification == BCType.TruePositive)
        fn_count = np.sum(ref_binary_classification == BCType.FalseNegative)
        tpr = tp_count / (tp_count + fn_count)
        return tpr