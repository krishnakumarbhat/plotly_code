import pytest

from aspe.evaluation.RadarObjectsEvaluation.Association.RelEstToRefNN import RelEstToRefNN
from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.BasicBinaryClassifier import \
    BasicBinaryClassifier
from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.IBinaryClassifier import BCType
from aspe.evaluation.RadarObjectsEvaluation.Flags.DistanceToCSOriginBelowThrFlag import \
    DistanceToCSOriginBelowThrFlag
from aspe.evaluation.RadarObjectsEvaluation.Flags.IsMovableFlag import IsMovableFlag
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.conversions import \
    convert_test_vector_to_pytest_param
from aspe.evaluation.RadarObjectsEvaluation.tests.artificial_data.relevancy_handling_data import \
    get_relevancy_handling_data

relevancy_handling_data = convert_test_vector_to_pytest_param(get_relevancy_handling_data())


class TestFunctionalPEPipelineBasicBinaryClassification:

    @pytest.fixture
    def pe_pipeline_configuration(self):
        pe_pipeline = PEPipeline(relevancy_estimated_objects=IsMovableFlag(),
                                 relevancy_reference_objects=DistanceToCSOriginBelowThrFlag(thr=90.0),
                                 association=RelEstToRefNN(),
                                 binary_classification=BasicBinaryClassifier())
        return pe_pipeline

    @pytest.mark.parametrize('estimated_data, reference_data', relevancy_handling_data)
    def test_if_ref_not_rel_bc_is_nan(self, pe_pipeline_configuration, estimated_data, reference_data):
        """
        """
        # Setup
        pe = pe_pipeline_configuration

        # Expected
        expected_all_results = True

        # Evaluate
        pe_output = pe.evaluate_single_data(estimated_data=estimated_data,
                                            reference_data=reference_data)

        # Verify
        ref_rel_flag_mask = pe_output.pe_results_obj_ref.signals['relevancy_flag']

        ref_mask = ~ref_rel_flag_mask
        bc_mask = pe_output.pe_results_obj_ref.signals['binary_classification'] == BCType.NotAssigned

        observed_all_results = (bc_mask == ref_mask).all()

        assert (observed_all_results == expected_all_results)

    @pytest.mark.parametrize('estimated_data, reference_data', relevancy_handling_data)
    def test_if_est_not_rel_and_associated_bc_is_nan(self, pe_pipeline_configuration, estimated_data, reference_data):
        """
        """
        # Setup
        pe = pe_pipeline_configuration

        # Expected
        expected_all_results = True

        # Evaluate
        pe_output = pe.evaluate_single_data(estimated_data=estimated_data,
                                            reference_data=reference_data)

        # Verify
        est_rel_flag_mask = pe_output.pe_results_obj_est.signals['relevancy_flag']
        est_asso_flag_mask = pe_output.pe_results_obj_est.signals['is_associated']

        est_mask = ~est_rel_flag_mask & est_asso_flag_mask
        bc_mask = pe_output.pe_results_obj_est.signals['binary_classification'] == BCType.NotAssigned

        observed_all_results = bc_mask[est_mask].all()

        assert (observed_all_results == expected_all_results)

    @pytest.mark.parametrize('estimated_data, reference_data', relevancy_handling_data)
    def test_if_ref_rel_and_not_associated_bc_is_fn(self, pe_pipeline_configuration, estimated_data, reference_data):
        """
        """
        # Setup
        pe = pe_pipeline_configuration

        # Expected
        expected_all_results = True

        # Evaluate
        pe_output = pe.evaluate_single_data(estimated_data=estimated_data,
                                            reference_data=reference_data)

        # Verify
        ref_rel_flag_mask = pe_output.pe_results_obj_ref.signals['relevancy_flag']
        ref_asso_flag_mask = pe_output.pe_results_obj_ref.signals['is_associated']

        ref_mask = ref_rel_flag_mask & ~ref_asso_flag_mask
        bc_mask = pe_output.pe_results_obj_ref.signals['binary_classification'] == BCType.FalseNegative

        observed_all_results = (bc_mask == ref_mask).all()

        assert (observed_all_results == expected_all_results)
