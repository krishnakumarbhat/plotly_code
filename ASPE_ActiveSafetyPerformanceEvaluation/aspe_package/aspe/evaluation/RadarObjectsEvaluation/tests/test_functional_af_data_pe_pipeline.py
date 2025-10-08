import pytest

from aspe.evaluation.RadarObjectsEvaluation.Flags.AllTrueFlag import AllTrueFlag
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.conversions import \
    convert_test_vector_to_pytest_param, convert_test_vector_to_pytest_param_with_kpis
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.ideal_data import get_ideal_data
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.shifted_data import get_different_shifts
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.tp_fn_fp_data import \
    get_single_ref_tp_fn_fp_data

ideal_data = convert_test_vector_to_pytest_param(get_ideal_data())
different_shifts_data = convert_test_vector_to_pytest_param(get_different_shifts())
single_ref_tp_fp_fn_data_with_kpis = convert_test_vector_to_pytest_param_with_kpis(get_single_ref_tp_fn_fp_data())


class TestFunctionalPEPipeline:
    """
    Functional tests for PE Pipeline based on artificial data
    """

    @pytest.mark.parametrize('kpi', ['TPR', 'PPV'])
    @pytest.mark.parametrize('estimated_data, reference_data', ideal_data)
    def test_trp_ppv_on_ideal_data(self, estimated_data, reference_data, kpi):
        """
        Check initialization
        :return:
        """
        # Setup
        pe = PEPipeline(relevancy_estimated_objects=AllTrueFlag(),
                        relevancy_reference_objects=AllTrueFlag())

        # Expected
        expected_kpi = 1.0

        # Evaluate
        pe_output = pe.evaluate(estimated_data=estimated_data,
                                reference_data=reference_data)
        observed_kpi = pe_output.kpis_binary_class_per_log.set_index('signature').at[kpi, 'value']

        # Verify
        assert (observed_kpi == expected_kpi)

    @pytest.mark.parametrize('kpi_name', ['TPR', 'PPV'])
    @pytest.mark.parametrize('estimated_data, reference_data, expected_kpis', single_ref_tp_fp_fn_data_with_kpis)
    def test_trp_ppv_on_fp_fn_data(self, estimated_data, reference_data, expected_kpis, kpi_name):
        """
        Check TPR and PPV KPIs on test vectors containing single reference and several types of FP, FN
        :return:
        """
        # Setup
        pe = PEPipeline()
        allowed_diff = 1e-3

        # Expected
        expected_kpi = expected_kpis[kpi_name]

        # Evaluate
        pe_output = pe.evaluate(estimated_data=estimated_data,
                                reference_data=reference_data)
        observed_kpi = pe_output.kpis_binary_class_per_log.set_index('signature').at[kpi_name, 'value']
        
        # Verify
        assert (observed_kpi == pytest.approx(expected_kpi, allowed_diff))
