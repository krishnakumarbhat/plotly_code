"""
Unittests for SGSampleFeatures builder
"""
import pandas as pd
import pytest
from pandas.testing import assert_frame_equal

from aspe.evaluation.RadarSGEvaluation.Builders.SGSampleFeatures import get_sample_features
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSamples import SGSamples
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSampleFeatures import SGSampleFeatures
from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType
from aspe.evaluation.RadarSGEvaluation.Enums.SegmentationType import SegmentationType


@pytest.fixture()
def test_setup_1():
    samples_df = pd.DataFrame({
        'scan_index': [0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 4, 4, 4],
        'timestamp': [0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.4, 0.4, 0.4],
        'unique_id': [0, 2, 4, 6, 8, 11, 12, 15, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26],
        'segment_unique_id': [0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3],
        'sample_position_x': [0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.0, 1.0, 1.0, -3.0, -3.2, -3.4, -3.6, -3.8, -4.0, 1.5,
                              1.25, 1.0],
        'sample_position_y': [1.0, 1.4, 1.8, 2.2, 2.6, 3.0, 3.0, 3.5, 4.0, -2.0, -2.4, -2.8, -3.2, -3.6, -4.0, -1.5,
                              -1.25, -1.0],
        'projection_sample_position_x': [0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.0, 1.0, 1.0, -3.0, -3.2, -3.4, -3.6, -3.8,
                                         -4.0, 1.5, 1.25, 1.0],
        'projection_sample_position_y': [1.0, 1.4, 1.8, 2.2, 2.6, 3.0, 3.0, 3.5, 4.0, -2.0, -2.4, -2.8, -3.2, -3.6,
                                         -4.0, -1.5, -1.25, -1.0],
        'distance': 18 * [0.0],
        'deviation_x': 18 * [0.0],
        'deviation_y': 18 * [0.0],
        'classification': 18 * [True],
        'multiple_segmentation': [False, False, False, False, False, True, True, False, False, False, False, False,
                                  False, False, False, False, False, False],
    })
    samples = SGSamples()
    samples.insert_samples(samples_df)
    expected_output_df = pd.DataFrame({
        'scan_index': [0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 4, 4, 4],
        'timestamp': [0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.4, 0.4, 0.4],
        'unique_id': [0, 2, 4, 6, 8, 11, 12, 15, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26],
        'distance': 18 * [0.0],
        'deviation_x': 18 * [0.0],
        'deviation_y': 18 * [0.0],
        'binary_classification': 18 * [BCType.TruePositive],
        'segmentation_type': [SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.UnderSegmentation,
                              SegmentationType.UnderSegmentation, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned],
    })
    expected_output = SGSampleFeatures()
    expected_output.insert_feature(expected_output_df)
    return samples, expected_output


def test_SGSampleFeatures_builder_1(test_setup_1):
    samples, expected_output = test_setup_1
    output_samples = get_sample_features(samples.signals, BCType.FalsePositive, SegmentationType.UnderSegmentation)

    assert assert_frame_equal(output_samples.signals, expected_output.signals) is None


@pytest.fixture()
def test_setup_2():
    samples_df = pd.DataFrame({
        'scan_index': [0, 0],
        'timestamp': [0.1, 0.1],
        'unique_id': [0, 2],
        'segment_unique_id': [0, 0],
        'sample_position_x': [0.0, 0.2],
        'sample_position_y': [1.0, 1.4],
        'projection_sample_position_x': [0.0, 0.2],
        'projection_sample_position_y': [1.0, 1.4],
        'distance': [0.0, 0.0],
        'deviation_x': [0.0, 0.0],
        'deviation_y': [0.0, 0.0],
        'classification': [True, False],
        'multiple_segmentation': [False, True],
    })
    samples = SGSamples()
    samples.insert_samples(samples_df)
    expected_output_df = pd.DataFrame({
        'scan_index': [0, 0],
        'timestamp': [0.1, 0.1],
        'unique_id': [0, 2],
        'distance': [0.0, 0.0],
        'deviation_x': [0.0, 0.0],
        'deviation_y': [0.0, 0.0],
        'binary_classification': [BCType.TruePositive, BCType.FalseNegative],
        'segmentation_type': [SegmentationType.NotAssigned, SegmentationType.NotAssigned],
    })
    expected_output = SGSampleFeatures()
    expected_output.insert_feature(expected_output_df)
    return samples, expected_output


def test_SGSampleFeatures_builder_2(test_setup_2):
    samples, expected_output = test_setup_2
    output_samples = get_sample_features(samples.signals, BCType.FalseNegative, SegmentationType.OverSegmentation)
    assert assert_frame_equal(output_samples.signals, expected_output.signals) is None
