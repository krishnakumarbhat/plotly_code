import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.DistanceToCSOriginBelowThrFlag import DistanceToCSOriginBelowThrFlag
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.ref_test_vectors import get_ref_test_vectors

raw_data = get_ref_test_vectors()
test_vectors = dict()
for single_raw_data_name, single_raw_data in raw_data.items():
    test_vectors[single_raw_data_name] = single_raw_data.objects.signals


class TestDistanceToCSOriginBelowThrFlag:

    def test_within_fov(self):
        # Setup
        data = test_vectors['four_in_fov']
        distance_checker = DistanceToCSOriginBelowThrFlag(thr=100.0)

        # Expected
        expected_all_results = True

        # Evaluate
        observed_flag = distance_checker.calc_flag(data)
        observed_all_results = observed_flag.all()

        # Verify
        assert (expected_all_results == observed_all_results)

    def test_out_of_fov(self):
        # Setup
        data = test_vectors['two_out_of_fov']
        distance_checker = DistanceToCSOriginBelowThrFlag(thr=100.0)

        # Expected
        expected_any_results = False

        # Evaluate
        observed_flag = distance_checker.calc_flag(data)
        observed_any_results = observed_flag.any()

        # Verify
        assert (expected_any_results == observed_any_results)

    def test_entering_fov(self):
        # Setup
        data = test_vectors['single_entering_fov']
        distance_checker = DistanceToCSOriginBelowThrFlag(thr=100.0)

        # Expected
        expected_flag = pd.Series([False, False, False, True, True, True])

        # Evaluate
        observed_flag = distance_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)

    def test_small_distance(self):
        # Setup
        data = test_vectors['single_long_leaving']
        distance_checker = DistanceToCSOriginBelowThrFlag(thr=8.0)

        # Expected
        expected_flag = pd.Series([True, True, True, True, True, True, False, False, False, False, False])

        # Evaluate
        observed_flag = distance_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)



