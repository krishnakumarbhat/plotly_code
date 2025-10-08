import pytest

from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.conversions import \
    convert_test_vector_to_pytest_param
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.ideal_data import get_ideal_data
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.shifted_data import get_different_shifts
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.tp_fn_fp_data import \
    get_single_ref_tp_fn_fp_data

ideal_data = convert_test_vector_to_pytest_param(get_ideal_data())
different_shifts_data = convert_test_vector_to_pytest_param(get_different_shifts())
single_ref_tp_fp_fn_data = convert_test_vector_to_pytest_param(get_single_ref_tp_fn_fp_data())


class TestSmokePEPipeline:
    """
    Smoke test class for pipeline functionalists
    """

    def test_smoke_init_pipeline(self):
        """
        Check initialization
        :return:
        """
        PEPipeline()

    @pytest.mark.parametrize('estimated_data, reference_data', ideal_data)
    def test_smoke_call_pipeline_ideal_data(self, estimated_data, reference_data):
        """
        Check main pipeline method
        :return:
        """
        pe = PEPipeline()
        pe.evaluate(estimated_data=estimated_data,
                    reference_data=reference_data)

    @pytest.mark.parametrize('estimated_data, reference_data', different_shifts_data)
    def test_smoke_call_pipeline_different_shifts_data(self, estimated_data, reference_data):
        """
        Check main pipeline method
        :return:
        """
        pe = PEPipeline()
        pe.evaluate(estimated_data=estimated_data,
                    reference_data=reference_data)

    @pytest.mark.parametrize('estimated_data, reference_data', single_ref_tp_fp_fn_data)
    def test_smoke_call_pipeline_single_ref_tp_fp_fn_data(self, estimated_data, reference_data):
        """
        Check main pipeline method
        :return:
        """
        pe = PEPipeline()
        pe.evaluate(estimated_data=estimated_data,
                    reference_data=reference_data)
