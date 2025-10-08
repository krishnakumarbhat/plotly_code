import pytest
from pandas import DataFrame, testing

from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSampleFeatures import SGSampleFeatures
from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType
from aspe.evaluation.RadarSGEvaluation.Enums.SegmentationType import SegmentationType


def test_SGSampleFeatures_insert_feature_valid_data():
    expected_signals_df = DataFrame.from_dict({
        'scan_index': {0: 0}, 'timestamp': {0: 0}, 'unique_id': {0: 1},
        'distance': {0: 1.0}, 'deviation_x': {0: 1.0},
        'deviation_y': {0: 1.0}, 'binary_classification': {0: BCType.NotAssigned},
        'segmentation_type': {0: SegmentationType.NotAssigned}})
    sg_samples_features = SGSampleFeatures()
    sg_samples_features.insert_feature(expected_signals_df)
    assert testing.assert_frame_equal(expected_signals_df, sg_samples_features.signals) is None


def test_SGSampleFeatures_insert_feature_invalid_data():
    error_msg = "Missing columns inside SGSampleFeatures dataframe!"
    with pytest.raises(AttributeError, match=error_msg):
        expected_signals_df = DataFrame.from_dict(
            {'scan_index': {0: 0}, 'timestamp': {0: 0}, 'unique_id': {0: 1},
             'distance_cart': {0: 1.0}, 'deviation_x': {0: 1.0},
             'deviation_y': {0: 1.0}, 'binary_classification': {0: BCType.NotAssigned},
             'segmentation_type': {0: SegmentationType.NotAssigned}})
        sg_samples_features = SGSampleFeatures()
        sg_samples_features.insert_feature(expected_signals_df)


def test_SGSampleFeatures_get_base_name():
    assert 'SGSampleFeatures' == SGSampleFeatures().get_base_name()
