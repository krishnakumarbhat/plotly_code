import pytest
import numpy as np

from aspe.evaluation.RadarDetectionEvaluation.DetectionEvaluation import DetectionEvaluation
from aspe.evaluation.RadarDetectionEvaluation.PreProcesors.MudpPreProcessor import MudpPreProcessor
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors import default_data
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.angle_jumps_data import \
    get_angle_jumps_from_overtaking_object, get_angle_jumps_small_rel_speed_object, get_angle_jumps_same_speed_object
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.complex_data import \
    get_two_objects_with_range_rate_conflicts, get_three_objects_with_different_features
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.ideal_data import \
    get_single_object_with_single_ideal_detection, get_single_object_with_three_ideal_detection, \
    get_single_object_with_three_ideal_detection_inside, get_two_object_with_three_ideal_detection_inside_each
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.support_functions import \
    convert_test_vector_to_pytest_param_est_ref, convert_test_vector_to_pytest_param_est_ref_exp

ideal_data = convert_test_vector_to_pytest_param_est_ref([get_single_object_with_single_ideal_detection(),
                                                          get_single_object_with_three_ideal_detection(),
                                                          get_single_object_with_three_ideal_detection_inside(),
                                                          get_two_object_with_three_ideal_detection_inside_each()])

number_of_associated_data = convert_test_vector_to_pytest_param_est_ref_exp([get_two_objects_with_range_rate_conflicts(),
                                                                             get_three_objects_with_different_features(),
                                                                             get_angle_jumps_from_overtaking_object(),
                                                                             get_angle_jumps_small_rel_speed_object(),
                                                                             get_angle_jumps_same_speed_object()])

association_pairs_data = convert_test_vector_to_pytest_param_est_ref_exp([get_three_objects_with_different_features(),
                                                                          get_angle_jumps_small_rel_speed_object(),
                                                                          get_angle_jumps_same_speed_object()])


@pytest.fixture()
def default_evaluation():
    return DetectionEvaluation(pre_processor=MudpPreProcessor())


class TestDetectionEvaluationSingleSensor:
    @pytest.mark.parametrize('estimated_data, reference_data', ideal_data)
    def test_ideal_data(self, estimated_data, reference_data, default_evaluation):
        sensor_id = reference_data.sensors.per_sensor.loc[0, 'sensor_id']
        outputs = default_evaluation.evaluate_single_sensor(estimated_data, reference_data, sensor_id)

        pairs_df = outputs['pairs_df']
        f_associated = pairs_df['f_associated'].to_numpy()
        assert f_associated.all()

    @pytest.mark.parametrize('estimated_data, reference_data, expectation', number_of_associated_data)
    def test_number_of_associated(self, estimated_data, reference_data, expectation, default_evaluation):
        sensor_id = reference_data.sensors.per_sensor.loc[0, 'sensor_id']
        outputs = default_evaluation.evaluate_single_sensor(estimated_data, reference_data, sensor_id)

        pairs_df = outputs['pairs_df']
        observed_f_associated = pairs_df['f_associated'].to_numpy()
        observed_number_of_associated = observed_f_associated.sum()
        expected_number_of_associated = expectation['n_associated']
        assert observed_number_of_associated == expected_number_of_associated

    @pytest.mark.parametrize('estimated_data, reference_data, expectation', association_pairs_data)
    def test_association_pairs(self, estimated_data, reference_data, expectation, default_evaluation):
        sensor_id = reference_data.sensors.per_sensor.loc[0, 'sensor_id']
        outputs = default_evaluation.evaluate_single_sensor(estimated_data, reference_data, sensor_id)

        pairs_df = outputs['pairs_df']
        pairs_df_associated = pairs_df[pairs_df['f_associated']]
        pairs_df_associated.sort_values(['unique_id_obj', 'unique_id_det'], inplace=True)
        observed_associated_pairs = pairs_df_associated[['unique_id_obj', 'unique_id_det']].to_numpy().astype(int)
        expected_associated_pairs = expectation['associated_pairs']
        np.testing.assert_array_equal(observed_associated_pairs, expected_associated_pairs)



