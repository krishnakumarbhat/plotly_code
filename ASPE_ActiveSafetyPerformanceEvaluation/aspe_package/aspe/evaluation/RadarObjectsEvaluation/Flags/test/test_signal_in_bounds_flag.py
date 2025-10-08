import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.SignalInBoundsFlag import SignalInBoundsFlag
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.ref_test_vectors import get_ref_test_vectors

raw_data = get_ref_test_vectors()
test_vectors = dict()
for single_raw_data_name, single_raw_data in raw_data.items():
    test_vectors[single_raw_data_name] = single_raw_data.objects.signals


class TestSignalInBoundsFlagSpeed:

    def test_for_stopped_object(self):
        # Setup
        data = test_vectors['single_stopped_back']
        speed_checker = SignalInBoundsFlag(signal_name='speed', min_value=2.0)

        # Expected
        expected_flag = pd.Series([False, True, False])

        # Evaluate
        observed_flag = speed_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)

    def test_for_moving_object(self):
        # Setup
        data = test_vectors['single_moving_same_dir_front']
        speed_checker = SignalInBoundsFlag(signal_name='speed', min_value=2.0)

        # Expected
        expected_flag = pd.Series([True, True, True])

        # Evaluate
        observed_flag = speed_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)

    def test_for_moving_big_thr(self):
        # Setup
        data = test_vectors['single_moving_same_dir_front']
        speed_checker = SignalInBoundsFlag(signal_name='speed', min_value=100.0)

        # Expected
        expected_flag = pd.Series([False, False, False])

        # Evaluate
        observed_flag = speed_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)

    def test_for_stationary_object(self):
        # Setup
        data = test_vectors['single_stationary_front']
        speed_checker = SignalInBoundsFlag(signal_name='speed', min_value=2.0)

        # Expected
        expected_flag = pd.Series([False, False, False])

        # Evaluate
        observed_flag = speed_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)


class TestSignalInBoundsFlagPosition:

    def test_for_position_x(self):
        # Setup
        data = test_vectors['single_entering_fov']
        position_x_checker = SignalInBoundsFlag(signal_name='position_x', min_value=-100.0, max_value=100.0)

        # Expected
        expected_flag = pd.Series([False, False, True, True, True, True])

        # Evaluate
        observed_flag = position_x_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)

    def test_for_position_y(self):
        # Setup
        data = test_vectors['single_moving_crossing_turning']
        position_x_checker = SignalInBoundsFlag(signal_name='position_y', min_value=-18, max_value=100.0)

        # Expected
        expected_flag = pd.Series([False, False, True])

        # Evaluate
        observed_flag = position_x_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)

