import pytest
import numpy as np
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.intersection_over_union_ratio import *

reference_polygons = [Polygon([(0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 0.0)]),
                      Polygon([(0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 0.0)]),
                      Polygon([(0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 0.0)]),
                      Polygon([(0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 0.0)]),
                      Polygon([(0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 0.0)])]
estimated_polygons = [Polygon([(0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 0.0)]),
                      Polygon([(0.25, 0.25), (0.25, 0.75), (0.75, 0.75), (0.75, 0.25)]),
                      Polygon([(0.5, 0.75), (0.5, 1.75), (1.5, 1.75), (1.5, 0.75)]),
                      Polygon([(10.0, 10.0), (10.0, 11.0), (11.0, 11.0), (11.0, 10.0)]),
                      Polygon([(1.0, 0.0), (0.0, 1.0), (1.0, 2.0), (2.0, 1.0)])]

expected_IoU = [1.0, 0.25, 1.0 / 15, 0.0, 0.2]

test_data = list()
for i, each in enumerate(expected_IoU):
    single_data = pd.DataFrame({'ground_truth_bbox': [reference_polygons[i]], 'estimated_bbox': [estimated_polygons[i]]})
    test_data.append((single_data, pd.Series(expected_IoU[i])))

class TestCalculateIntersectionOverUnion:
    """
    Functional tests for Intersection over union calculations
    """

    @pytest.mark.parametrize('bboxes, expected_IoU', test_data)
    def test_calculate_intersection_over_union(self, bboxes, expected_IoU):
        """
        Check initialization
        :return:
        """
        # Setup
        IoU = IntersectionOverUnionRatio()

        # Evaluate
        IoU_output = IoU.calculate_intersection_over_union(bboxes['ground_truth_bbox'], bboxes['estimated_bbox'])

        # Verify
        assert (np.all(expected_IoU.eq(IoU_output)))
