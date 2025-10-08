import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.IBinaryClassifier import BCType
from aspe.evaluation.RadarObjectsEvaluation.DataStructures import PESingleObjects
from aspe.evaluation.RadarObjectsEvaluation.KPI.IBinaryClassificationKPI import IBinaryClassificationKPI


class OverSegmentationRatioKPI(IBinaryClassificationKPI):
    def __init__(self, *args, **kwargs):
        super().__init__('OversegmentationRatio')

    def calculate_kpi(self, reference_objects: PESingleObjects, estimated_objects: PESingleObjects):
        ref_binary_classification = reference_objects.signals.loc[:, 'binary_classification'].values
        oversegmentation_count = np.sum(ref_binary_classification == BCType.OverSegmentation)
        associated_num = np.sum(reference_objects.signals.loc[:, 'is_associated'])
        oversegmantation_ratio = oversegmentation_count / associated_num
        return oversegmantation_ratio