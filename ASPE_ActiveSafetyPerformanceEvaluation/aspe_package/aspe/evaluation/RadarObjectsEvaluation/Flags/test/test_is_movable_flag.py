import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IsMovableFlag import IsMovableFlag
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.ref_test_vectors import get_ref_test_vectors
from aspe.extractors.Interfaces.Enums.Object import MovementStatus

raw_data = get_ref_test_vectors()
test_vectors = dict()
for single_raw_data_name, single_raw_data in raw_data.items():
    test_vectors[single_raw_data_name] = single_raw_data.objects.signals


movement_status_test_vector = pd.DataFrame(
    data=[
        [1, 1000.43, 0.0, 0, 0, 0, 0, 0, MovementStatus.STATIONARY],
        [2, 1000.44, 1.0, 0, 0, 0, 0, 0, MovementStatus.MOVING],
        [3, 1000.45, 2.0, 0, 0, 0, 0, 0, MovementStatus.MOVING],
        [4, 1000.46, 3.0, 0, 0, 0, 0, 0, MovementStatus.MOVING],
        [5, 1000.47, 3.0, 0, 0, 0, 0, 0, MovementStatus.MOVING],
        [6, 1000.48, 2.0, 0, 0, 0, 0, 0, MovementStatus.STOPPED],
        [7, 1000.49, 1.0, 0, 0, 0, 0, 0, MovementStatus.UNDEFINED],
    ],
    columns=['scan_index', 'timestamp', 'speed', 'slot_id', 'unique_id',
             'center_x', 'center_y', 'bounding_box_orientation', 'movement_status'],
)


class TestIsMovableFlag:

    def test_for_stopped_object(self):
        # Setup
        data = test_vectors['single_stopped_back']
        movable_checker = IsMovableFlag()

        # Expected
        expected_flag = pd.Series([False, True, True])

        # Evaluate
        observed_flag = movable_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)

    def test_for_moving_object(self):
        # Setup
        data = test_vectors['single_moving_same_dir_front']
        movable_checker = IsMovableFlag()

        # Expected
        expected_flag = pd.Series([True, True, True])

        # Evaluate
        observed_flag = movable_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)

    def test_for_stationary_object(self):
        # Setup
        data = test_vectors['single_stationary_front']
        movable_checker = IsMovableFlag()

        # Expected
        expected_flag = pd.Series([False, False, False])

        # Evaluate
        observed_flag = movable_checker.calc_flag(data)

        # Verify
        assert expected_flag.equals(observed_flag)

    def test_for_movement_status_test_vector(self):
        # Setup
        movable_checker = IsMovableFlag()

        # Expected
        expected_flag = pd.Series([False, True, True, True, True, True, False])

        # Evaluate
        observed_flag = movable_checker.calc_flag(movement_status_test_vector)

        # Verify
        assert expected_flag.equals(observed_flag)

