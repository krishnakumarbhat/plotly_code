"""
Unittests for SGSegmentPairs builder.
"""
import pandas as pd
import pytest
from pandas.testing import assert_frame_equal

from aspe.evaluation.RadarSGEvaluation.Builders.SGSegmentPairs import build_pairs
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

    expected_output_df = pd.DataFrame({
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

    expected_output = SGSegmentPairs()
    expected_output.insert_pairs(expected_output_df)

    return segments, expected_output


def test_SGSegmentPairs_builder_same_data(test_same_segments_setup):
    segments, expected_output = test_same_segments_setup

    output_pairs = build_pairs(segments.signals, segments.signals, pairs_association_distance=2.0)

    assert assert_frame_equal(output_pairs[0].signals, expected_output.signals) is None


@pytest.fixture()
def test_same_segments_in_different_scans_setup():
    estimated_df = pd.DataFrame({'scan_index': [0, 0, 2, 4],
                                 'timestamp': [0.1, 0.1, 0.2, 0.4],
                                 'unique_id': [0, 1, 2, 3],
                                 'start_position_x': [0.0, 1.0, -3.0, 1.5],
                                 'start_position_y': [1.0, 3.0, -2.0, -1.5],
                                 'end_position_x': [1.0, 1.0, -4.0, 1.0],
                                 'end_position_y': [3.0, 4.0, -4.0, -1.0],
                                 'contour_unique_id': [1, 1, 5, 7],
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

    expected_output_df = pd.DataFrame({
        'scan_index': [2, 4],
        'timestamp': [0.2, 0.4],
        'unique_id': [0, 1],
        'segment_unique_id': [2, 3],
        'segment_unique_id_paired': [2, 3],
        'projection_start_position_x': [-3.0, 1.5],
        'projection_start_position_y': [-2.0, -1.5],
        'unit_vector_x_paired': [-0.4472136, -0.70710677],
        'unit_vector_y_paired': [-0.89443, 0.70711],
        'dot_product': [1.0, 1.0],
    })

    expected_output = SGSegmentPairs()
    expected_output.insert_pairs(expected_output_df)

    return segments_est, segments_ref, expected_output


def test_SGSegmentPairs_builder_1(test_same_segments_in_different_scans_setup):
    segments_est, segments_ref, expected_output = test_same_segments_in_different_scans_setup

    output_pairs = build_pairs(segments_est.signals, segments_ref.signals)

    assert assert_frame_equal(output_pairs[0].signals, expected_output.signals) is None


@pytest.fixture()
def test_all_from_first_scan_paired_setup():
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

    expected_output_df = pd.DataFrame({
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

    expected_output = SGSegmentPairs()
    expected_output.insert_pairs(expected_output_df)

    return segments_est, segments_ref, expected_output


def test_SGSegmentPairs_builder_2(test_all_from_first_scan_paired_setup):
    segments_est, segments_ref, expected_output = test_all_from_first_scan_paired_setup

    output_pairs = build_pairs(segments_est.signals, segments_ref.signals, pairs_association_distance=2.0)

    assert assert_frame_equal(output_pairs[0].signals, expected_output.signals) is None


@pytest.fixture()
def test_pair_found_in_every_scan():
    estimated_df = pd.DataFrame({
        'scan_index': [0, 0, 0, 1, 1],
        'timestamp': [0.1, 0.1, 0.1, 0.15, 0.15],
        'unique_id': [0, 1, 2, 3, 4],
        'start_position_x': [2.0, 2.0, 5.0, 1.0, 3.0],
        'start_position_y': [2.0, 4.0, 2.0, -4.0, -1.0],
        'end_position_x': [2.0, 3.0, 4.0, 3.0, 4.0],
        'end_position_y': [4.0, 6.0, 3.0, -1.0, -1.0],
        'contour_unique_id': [2, 2, 3, 4, 4],
    })
    segments_est = SGSegments()
    segments_est.insert_segments(estimated_df)

    reference_df = pd.DataFrame({
        'scan_index': [0, 1],
        'timestamp': [0.1, 0.15],
        'unique_id': [0, 1],
        'start_position_x': [2.0, 1.0],
        'start_position_y': [6.0, -4.0],
        'end_position_x': [3.0, 4.0],
        'end_position_y': [6.0, -1.0],
        'contour_unique_id': [5, 8],
    })
    segments_ref = SGSegments()
    segments_ref.insert_segments(reference_df)

    expected_output_df = pd.DataFrame({
        'scan_index': [0, 1, 1],
        'timestamp': [0.1, 0.15, 0.15],
        'unique_id': [0, 1, 2],
        'segment_unique_id': [1, 3, 4],
        'segment_unique_id_paired': [0, 1, 1],
        'projection_start_position_x': [-2.0, 1.0, 3.0],
        'projection_start_position_y': [6.0, -4.0, -2.0],
        'unit_vector_x_paired': [1.0, 0.70711, 0.70711],
        'unit_vector_y_paired': [0.0, 0.70711, 0.70711],
        'dot_product': [0.44721, 0.98058, 0.70711],
    })

    expected_output = SGSegmentPairs()
    expected_output.insert_pairs(expected_output_df)

    return segments_est, segments_ref, expected_output


def test_SGSegmentPairs_builder_3(test_pair_found_in_every_scan):
    segments_est, segments_ref, expected_output = test_pair_found_in_every_scan

    output_pairs = build_pairs(segments_est.signals, segments_ref.signals, pairs_association_distance=1.0)

    assert assert_frame_equal(output_pairs[0].signals, expected_output.signals) is None


@pytest.fixture()
def test_lack_of_same_scans():
    estimated_df = pd.DataFrame({
        'scan_index': [0],
        'timestamp': [0.1],
        'unique_id': [1],
        'start_position_x': [1.0],
        'start_position_y': [1.0],
        'end_position_x': [3.0],
        'end_position_y': [3.0],
        'contour_unique_id': [5],
    })
    segments_est = SGSegments()
    segments_est.insert_segments(estimated_df)

    reference_df = pd.DataFrame({
        'scan_index': [1],
        'timestamp': [0.1],
        'unique_id': [1],
        'start_position_x': [1.0],
        'start_position_y': [1.0],
        'end_position_x': [3.0],
        'end_position_y': [3.0],
        'contour_unique_id': [5],
    })
    segments_ref = SGSegments()
    segments_ref.insert_segments(reference_df)

    expected_output_df = pd.DataFrame({
        'scan_index': [],
        'timestamp': [],
        'unique_id': [],
        'segment_unique_id': [],
        'segment_unique_id_paired': [],
        'projection_start_position_x': [],
        'projection_start_position_y': [],
        'unit_vector_x_paired': [],
        'unit_vector_y_paired': [],
        'dot_product': [],
    })

    expected_output = SGSegmentPairs()
    expected_output.insert_pairs(expected_output_df)

    return segments_est, segments_ref, expected_output


def test_SGSegmentPairs_builder_lack_of_same_scans(test_lack_of_same_scans):
    segments_est, segments_ref, expected_output = test_lack_of_same_scans

    output_pairs = build_pairs(segments_est.signals, segments_ref.signals)

    assert assert_frame_equal(output_pairs[0].signals, expected_output.signals) is None


@pytest.fixture()
def test_lack_of_paired_segments():
    estimated_df = pd.DataFrame({
        'scan_index': [0],
        'timestamp': [0.1],
        'unique_id': [1],
        'start_position_x': [1.0],
        'start_position_y': [1.0],
        'end_position_x': [3.0],
        'end_position_y': [3.0],
        'contour_unique_id': [5],
    })
    segments_est = SGSegments()
    segments_est.insert_segments(estimated_df)

    reference_df = pd.DataFrame({
        'scan_index': [0],
        'timestamp': [0.1],
        'unique_id': [1],
        'start_position_x': [10.0],
        'start_position_y': [10.0],
        'end_position_x': [30.0],
        'end_position_y': [30.0],
        'contour_unique_id': [5],
    })
    segments_ref = SGSegments()
    segments_ref.insert_segments(reference_df)

    expected_output_df = pd.DataFrame({
        'scan_index': [],
        'timestamp': [],
        'unique_id': [],
        'segment_unique_id': [],
        'segment_unique_id_paired': [],
        'projection_start_position_x': [],
        'projection_start_position_y': [],
        'unit_vector_x_paired': [],
        'unit_vector_y_paired': [],
        'dot_product': [],
    })

    expected_output = SGSegmentPairs()
    expected_output.insert_pairs(expected_output_df)

    return segments_est, segments_ref, expected_output


def test_SGSegmentPairs_builder_lack_of_paired_segment(test_lack_of_paired_segments):
    segments_est, segments_ref, expected_output = test_lack_of_paired_segments

    output_pairs = build_pairs(segments_est.signals, segments_ref.signals, pairs_association_distance=1.0)

    assert assert_frame_equal(output_pairs[0].signals, expected_output.signals) is None
