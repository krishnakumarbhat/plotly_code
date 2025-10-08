import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.AllTrueFlag import AllTrueFlag


class TestAllTrueFlag:

    def test_positive(self):
        # Setup
        test_vector = pd.DataFrame(
            data=range(1, 100),
            columns=['c1'],
        )
        moving_checker = AllTrueFlag()

        # Evaluate
        observed_flag = moving_checker.calc_flag(test_vector)

        # Verify
        assert observed_flag.all()

    def test_negative(self):
        # Setup
        test_vector = pd.DataFrame(
            data=[],
            columns=['c1'],
        )
        moving_checker = AllTrueFlag()

        # Expected
        all_results = True

        # Evaluate
        observed_flag = moving_checker.calc_flag(test_vector)

        # Verify
        observed = observed_flag.all()
        assert (observed == all_results)

