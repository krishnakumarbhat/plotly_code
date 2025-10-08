from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEObjectsEvaluationOutput import \
    PEObjectsEvaluationOutputSingleLog
from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEObjectsEvaluationOutput import \
    PEObjectsEvaluationOutputMultiLog


class TestSmokeSingleLogOutput:
    def test_init_default_params(self):
        PEObjectsEvaluationOutputSingleLog()

class TestSmokeMultiLogResults:
    def test_init_default_params(self):
        PEObjectsEvaluationOutputMultiLog()

    def test_update_with_empty_structs(self):
        multi_results = PEObjectsEvaluationOutputMultiLog()
        single_results = PEObjectsEvaluationOutputSingleLog()

        multi_results.update('dummy_path', single_results)
