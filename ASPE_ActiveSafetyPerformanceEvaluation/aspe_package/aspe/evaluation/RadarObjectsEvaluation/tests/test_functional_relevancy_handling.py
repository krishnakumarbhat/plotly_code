import pytest

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
import numpy as np

relevancy_handling_data = convert_test_vector_to_pytest_param(get_relevancy_handling_data())


class TestFunctionalPEPipelineRelevancyHandling:

    @pytest.fixture
    def pe_pipeline_configuration(self):
        pe_pipeline = PEPipeline(relevancy_estimated_objects=IsMovableFlag(),
                                 relevancy_reference_objects=DistanceToCSOriginBelowThrFlag(thr=90.0),
                                 binary_classification=BasicBinaryClassifier())
        return pe_pipeline

    @pytest.mark.parametrize('estimated_data, reference_data', relevancy_handling_data)
    def test_if_not_relevant_est_objects_have_none_bc_status(self, pe_pipeline_configuration, estimated_data, reference_data):
        """
        """
        # Setup
        pe = pe_pipeline_configuration

        # Expected
        expected_all_results = True

        # Evaluate
        pe_output = pe.evaluate(estimated_data=estimated_data,
                                reference_data=reference_data)

        # Verify
        f_not_relevant_objects = np.logical_not(pe_output.pe_results_obj_est.signals.relevancy_flag.values)
        valid_data = pe_output.pe_results_obj_est.signals.loc[f_not_relevant_objects, :]

        bc_none = valid_data.binary_classification.values == BCType.NotAssigned
        observed_all_results = bc_none.all()

        assert (observed_all_results == expected_all_results)

    @pytest.mark.parametrize('estimated_data, reference_data', relevancy_handling_data)
    def test_if_not_relevant_ref_objects_have_none_bc_status(self, pe_pipeline_configuration, estimated_data, reference_data):
        """
        """
        # Setup
        pe = pe_pipeline_configuration

        # Expected
        expected_all_results = True

        # Evaluate
        pe_output = pe.evaluate(estimated_data=estimated_data,
                                reference_data=reference_data)

        # Verify
        f_not_relevant_objects = np.logical_not(pe_output.pe_results_obj_ref.signals.relevancy_flag.values)
        valid_data = pe_output.pe_results_obj_ref.signals.loc[f_not_relevant_objects, :]

        bc_none = valid_data.binary_classification.values == BCType.NotAssigned
        observed_all_results = bc_none.all()

        assert (observed_all_results == expected_all_results)

    @pytest.mark.parametrize('estimated_data, reference_data', relevancy_handling_data)
    def test_if_not_relevant_pairs_have_none_bc_status(self, pe_pipeline_configuration, estimated_data, reference_data):
        """
        """
        # Setup
        pe = pe_pipeline_configuration

        # Expected
        expected_all_results = True

        # Evaluate
        pe_output = pe.evaluate(estimated_data=estimated_data,
                                reference_data=reference_data)

        # Verify
        not_relevancy_est_flag = np.logical_not(pe_output.pe_results_obj_pairs.signals.relevancy_flag_est.values)
        not_relevancy_ref_flag = np.logical_not(pe_output.pe_results_obj_pairs.signals.relevancy_flag_ref.values)
        valid_data_mask = np.logical_or(not_relevancy_est_flag, not_relevancy_ref_flag)
        valid_data = pe_output.pe_results_obj_pairs.signals.loc[valid_data_mask, :]

        bc_valid = valid_data.binary_classification.values == BCType.NotAssigned
        observed_all_results = bc_valid.all()

        assert (observed_all_results == expected_all_results)

    @pytest.mark.parametrize('estimated_data, reference_data', relevancy_handling_data)
    def test_if_relevant_pairs_have_valid_bc_status(self, pe_pipeline_configuration, estimated_data, reference_data):
        """
        Check if object which are not relevant has binary classification status set to none
        :param estimated_data:
        :param reference_data:
        :return:
        """
        # Setup
        pe = pe_pipeline_configuration

        # Expected
        expected_all_results = True

        # Evaluate
        pe_output = pe.evaluate(estimated_data=estimated_data,
                                reference_data=reference_data)

        # Verify
        relevancy_est_flag = pe_output.pe_results_obj_pairs.signals.relevancy_flag_est.values
        relevancy_ref_flag = pe_output.pe_results_obj_pairs.signals.relevancy_flag_ref.values
        valid_data_mask = np.logical_and(relevancy_est_flag, relevancy_ref_flag)
        valid_data = pe_output.pe_results_obj_pairs.signals.loc[valid_data_mask, :]

        bc_valid = valid_data.binary_classification.values != BCType.NotAssigned
        observed_all_results = bc_valid.all()

        assert (observed_all_results == expected_all_results)