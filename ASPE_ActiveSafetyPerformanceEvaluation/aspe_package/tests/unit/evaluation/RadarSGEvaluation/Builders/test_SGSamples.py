"""
Unittests for SGSamples builder
"""
import numpy as np
import pandas as pd
import pytest
from pandas.testing import assert_frame_equal

from aspe.evaluation.RadarSGEvaluation.Builders.SGSamples import associate_samples
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSamples import SGSamples
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSegments import SGSegments
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSegmentPairs import SGSegmentPairs


@pytest.fixture()
def test_same_segments_setup():
    df = pd.DataFrame({'scan_index': [0, 0, 2, 4],
                       'timestamp': [0.1, 0.1, 0.2, 0.4],
                       'unique_id': [0, 1, 2, 3],
                       'start_position_x': [0.0, 1.0, -3.0, 1.5],
                       'start_position_y': [1.0, 3.0, -2.0, -1.5],
                       'end_position_x': [1.0, 1.0, -4.0, 1.0],
                       'end_position_y': [3.0, 4.0, -4.0, -1.0],
                       'contour_unique_id': [1, 1, 5, 7],
                       })
    segments = SGSegments()
    segments.insert_segments(df)
    segment_pairs_df = pd.DataFrame({
        'scan_index': [0, 0, 0, 0, 2, 4],
        'timestamp': [0.1, 0.1, 0.1, 0.1, 0.2, 0.4],
        'unique_id': [0, 1, 2, 3, 4, 5],
        'segment_unique_id': [0, 0, 1, 1, 2, 3],
        'segment_unique_id_paired': [0, 1, 0, 1, 2, 3],
        'projection_start_position_x': [0, 1.0, 1.0, 1.0, -3.0, 1.5],
        'projection_start_position_y': [1.0, 0.5, 3.0, 3.0, -2.0, -1.5],
        'unit_vector_x_paired': [0.44721, 0.0, -0.44721, 0.0, -0.44721, -0.70711],
        'unit_vector_y_paired': [0.89443, 1.0, -0.89443, 1.0, -0.89443, 0.70711],
        'dot_product': [1.0, 0.8944272, -0.8944272, 1.0, 1.0, 1.0],
    })
    segment_pairs = SGSegmentPairs()
    segment_pairs.insert_pairs(segment_pairs_df)
    expected_output_df = pd.DataFrame({
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
    expected_output = SGSamples()
    expected_output.insert_samples(expected_output_df)
    return segments, segment_pairs, expected_output


def test_SGSamples_builder_same_data(test_same_segments_setup):
    segments, segment_pairs, expected_output = test_same_segments_setup
    output_samples = associate_samples(segments.signals, segments.signals, segment_pairs.signals)
    output_samples.signals.sort_values('unique_id', inplace=True)
    output_samples.signals.reset_index(inplace=True)
    output_samples.signals.drop('index', axis=1, inplace=True)
    assert assert_frame_equal(output_samples.signals, expected_output.signals, check_dtype=False, rtol=1e-3,
                              atol=1e-3) is None


@pytest.fixture()
def test_only_first_scan_paired_setup():
    estimated_df = pd.DataFrame({'scan_index': [0, 0, 1],
                                 'timestamp': [0.0, 0.0, 0.1],
                                 'unique_id': [0, 1, 2],
                                 'start_position_x': [0.0, 1.0, 1.0],
                                 'start_position_y': [1.0, 3.0, 1.0],
                                 'end_position_x': [1.0, 1.0, 2.0],
                                 'end_position_y': [3.0, 4.0, 3.0],
                                 'contour_unique_id': [1, 1, 5],
                                 })
    segments_est = SGSegments()
    segments_est.insert_segments(estimated_df)
    reference_df = pd.DataFrame({'scan_index': [1, 1, 2, 4],
                                 'timestamp': [0.1, 0.1, 0.2, 0.4],
                                 'unique_id': [0, 1, 2, 3],
                                 'start_position_x': [0.0, 1.0, -3.0, 1.5],
                                 'start_position_y': [1.0, 3.0, -2.0, -1.5],
                                 'end_position_x': [1.0, 1.0, -4.0, 1.0],
                                 'end_position_y': [3.0, 4.0, -4.0, -1.0],
                                 'contour_unique_id': [1, 1, 5, 7],
                                 })
    segments_ref = SGSegments()
    segments_ref.insert_segments(reference_df)
    segment_pairs_df = pd.DataFrame({
        'scan_index': [1, 1],
        'timestamp': [0.1, 0.1],
        'unique_id': [0, 1],
        'segment_unique_id': [2, 2],
        'segment_unique_id_paired': [0, 1],
        'projection_start_position_x': [0.2, 1.0],
        'projection_start_position_y': [1.4, 1.0],
        'unit_vector_x_paired': [0.44721, 0.0],
        'unit_vector_y_paired': [0.8944272, 1.0],
        'dot_product': [1.0, 0.89443],
    })
    segment_pairs = SGSegmentPairs()
    segment_pairs.insert_pairs(segment_pairs_df)
    expected_output_df = pd.DataFrame({
        'scan_index': [0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1],
        'timestamp': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1],
        'unique_id': [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 18, 20],
        'segment_unique_id': [0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 2],
        'sample_position_x': [0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.0, 1.0, 1.0, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0],
        'sample_position_y': [1.0, 1.4, 1.8, 2.2, 2.6, 3.0, 3.0, 3.5, 4.0, 1.0, 1.4, 1.8, 2.2, 2.6, 3.0],
        'projection_sample_position_x': [np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, 0.2,
                                         0.4, 0.6, 0.8, 1.0, 1.0],
        'projection_sample_position_y': [np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, 1.4,
                                         1.8, 2.2, 2.6, 3.0, 3.5],
        'distance': [np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, 0.8944, 0.8944, 0.8944,
                     0.8944,
                     0.8944, 1.1180],
        'deviation_x': [np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, 0.8, 0.8, 0.8, 0.8, 0.8,
                        1.0],
        'deviation_y': [np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, np.nan, -0.4, -0.4, -0.4, -0.4,
                        -0.4, -0.5],
        'classification': [False, False, False, False, False, False, False, False, False, True, True, True, True, True,
                           True],
        'multiple_segmentation': [False, False, False, False, False, True, True, False, False, False, False, False,
                                  False, True, False],
    })
    expected_output = SGSamples()
    expected_output.insert_samples(expected_output_df)
    return segments_est, segments_ref, segment_pairs, expected_output


def test_SGSamples_builder_one_pair(test_only_first_scan_paired_setup):
    segments_est, segments_ref, segment_pairs, expected_output = test_only_first_scan_paired_setup
    output_samples = associate_samples(segments_est.signals, segments_ref.signals, segment_pairs.signals)
    output_samples.signals.sort_values('unique_id', inplace=True)
    output_samples.signals.reset_index(inplace=True)
    output_samples.signals.drop('index', axis=1, inplace=True)
    assert assert_frame_equal(output_samples.signals, expected_output.signals, check_dtype=False, rtol=1e-3,
                              atol=1e-3) is None


@pytest.fixture()
def pair_crossing_segments_setup():
    estimated_df = pd.DataFrame({
        'scan_index': [0],
        'timestamp': [0.0],
        'unique_id': [0],
        'start_position_x': [2.0],
        'start_position_y': [1.0],
        'end_position_x': [4.0],
        'end_position_y': [3.0],
        'contour_unique_id': [2],
    })
    segments_est = SGSegments()
    segments_est.insert_segments(estimated_df)
    reference_df = pd.DataFrame({
        'scan_index': [0],
        'timestamp': [0.0],
        'unique_id': [6],
        'start_position_x': [2.0],
        'start_position_y': [2.0],
        'end_position_x': [5.0],
        'end_position_y': [2.0],
        'contour_unique_id': [1],
    })
    segments_ref = SGSegments()
    segments_ref.insert_segments(reference_df)
    segment_pairs_df = pd.DataFrame({
        'scan_index': [0],
        'timestamp': [0.0],
        'unique_id': [0],
        'segment_unique_id': [6],
        'segment_unique_id_paired': [0],
        'projection_start_position_x': [2.0],
        'projection_start_position_y': [1.0],
        'unit_vector_x_paired': [0.70711],
        'unit_vector_y_paired': [0.70711],
        'dot_product': [0.70711],
    })
    segment_pairs = SGSegmentPairs()
    segment_pairs.insert_pairs(segment_pairs_df)
    expected_output_df = pd.DataFrame({
        'scan_index': 7 * [0],
        'timestamp': 7 * [0.0],
        'unique_id': [0, 1, 2, 3, 4, 5, 6],
        'segment_unique_id': 7 * [6],
        'sample_position_x': [2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0],
        'sample_position_y': 7 * [2.0],
        'projection_sample_position_x': [2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0],
        'projection_sample_position_y': [1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0],
        'distance': [1.0, 0.5, 0.0, 0.5, 1.0, 1.5, 2.0],
        'deviation_x': 7 * [0.0],
        'deviation_y': [1.0, 0.5, 0.0, -0.5, -1.0, -1.5, -2.0],
        'classification': [True, True, True, True, True, False, False],
        'multiple_segmentation': 7 * [False],
    })
    expected_output = SGSamples()
    expected_output.insert_samples(expected_output_df)
    return segments_est, segments_ref, segment_pairs, expected_output


def test_SGSamples_builder_crossing_segments(pair_crossing_segments_setup):
    segments_est, segments_ref, segment_pairs, expected_output = pair_crossing_segments_setup
    output_samples = associate_samples(segments_ref.signals, segments_est.signals, segment_pairs.signals)
    output_samples.signals.sort_values('unique_id', inplace=True)
    output_samples.signals.reset_index(inplace=True)
    output_samples.signals.drop('index', axis=1, inplace=True)
    assert assert_frame_equal(output_samples.signals, expected_output.signals, check_dtype=False, rtol=1e-3,
                              atol=1e-3) is None


@pytest.fixture()
def multiple_true_positives_setup():
    estimated_df = pd.DataFrame({
        'scan_index': [0],
        'timestamp': [0.0],
        'unique_id': [0],
        'start_position_x': [1.0],
        'start_position_y': [1.0],
        'end_position_x': [2.0],
        'end_position_y': [1.0],
        'contour_unique_id': [2],
    })
    segments_est = SGSegments()
    segments_est.insert_segments(estimated_df)
    reference_df = pd.DataFrame({
        'scan_index': [0, 0],
        'timestamp': [0.0, 0.0],
        'unique_id': [6, 7],
        'start_position_x': [1.0, 1.0],
        'start_position_y': [0.0, 1.5],
        'end_position_x': [2.0, 2.0],
        'end_position_y': [0.0, 1.5],
        'contour_unique_id': [1, 2],
    })
    segments_ref = SGSegments()
    segments_ref.insert_segments(reference_df)

    segment_pairs_df = pd.DataFrame({
        'scan_index': [0, 0],
        'timestamp': [0.0, 0.0],
        'unique_id': [0, 1],
        'segment_unique_id': [0, 0],
        'segment_unique_id_paired': [6, 7],
        'projection_start_position_x': [1.0, 1.0],
        'projection_start_position_y': [0.0, 1.5],
        'unit_vector_x_paired': [1.0, 1.0],
        'unit_vector_y_paired': [0.0, 0.0],
        'dot_product': [1.0, 1.0],
    })
    segment_pairs = SGSegmentPairs()
    segment_pairs.insert_pairs(segment_pairs_df)
    expected_output_df = pd.DataFrame({
        'scan_index': 3 * [0],
        'timestamp': 3 * [0.0],
        'unique_id': [1, 3, 5],
        'segment_unique_id': 3 * [0],
        'sample_position_x': [1.0, 1.5, 2.0],
        'sample_position_y': 3 * [1.0],
        'projection_sample_position_x': [1.0, 1.5, 2.0],
        'projection_sample_position_y': [1.5, 1.5, 1.5],
        'distance': 3 * [0.5],
        'deviation_x': 3 * [0.0],
        'deviation_y': 3 * [-0.5],
        'classification': 3 * [True],
        'multiple_segmentation': 3 * [False],
    })
    expected_output = SGSamples()
    expected_output.insert_samples(expected_output_df)
    return segments_est, segments_ref, segment_pairs, expected_output


def test_SGSamples_builder_multiple_true_positives(multiple_true_positives_setup):
    segments_est, segments_ref, segment_pairs, expected_output = multiple_true_positives_setup
    output_samples = associate_samples(segments_est.signals, segments_ref.signals, segment_pairs.signals,
                                       distance_threshold=0.75)
    output_samples.signals.sort_values('unique_id', inplace=True)
    output_samples.signals.reset_index(inplace=True)
    output_samples.signals.drop('index', axis=1, inplace=True)
    assert assert_frame_equal(output_samples.signals, expected_output.signals, check_dtype=False, rtol=1e-3,
                              atol=1e-3) is None
