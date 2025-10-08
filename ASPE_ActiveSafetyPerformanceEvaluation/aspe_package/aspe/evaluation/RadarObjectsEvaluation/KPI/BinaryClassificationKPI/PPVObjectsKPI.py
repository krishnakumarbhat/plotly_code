import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.IBinaryClassifier import BCType
from aspe.evaluation.RadarObjectsEvaluation.DataStructures import PESingleObjects
from aspe.evaluation.RadarObjectsEvaluation.KPI.IBinaryClassificationKPI import IBinaryClassificationKPI


class PPVBinaryClassificationKPI(IBinaryClassificationKPI):
    def __init__(self, *args, **kwargs):
        super().__init__('PPV')

    def calculate_kpi(self, reference_objects: PESingleObjects, estimated_objects: PESingleObjects):
        est_binary_classification = estimated_objects.signals.loc[:, 'binary_classification'].values
        tp_count = np.sum(est_binary_classification == BCType.TruePositive)
        fp_count = np.sum(est_binary_classification == BCType.FalsePositive)
        ppv = tp_count / (tp_count + fp_count)
        return ppv