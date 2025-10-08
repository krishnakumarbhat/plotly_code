import numpy as np
import pandas as pd
import shapely as sh

import pytest
from pandas.testing import assert_frame_equal

from aspe.evaluation.RadarSGEvaluation.SGEvaluationDraftMIA import SGEvaluation


@pytest.fixture()
def test_parallel_setup():
    estimated_contours = pd.DataFrame(
        {
            'scan_index': [0, 0],
            'unique_id': [1, 1],
            'position_x': [1, 3],
            'position_y': [1, 1],
        })

    reference_contours = pd.DataFrame(
        {
            'scan_index': [0, 0],
            'unique_id': [3, 3],
            'position_x': [2, 4],
            'position_y': [2, 2],
        })

    expected_estimate_evaluated = pd.DataFrame({
        'scan_index': 11 * [0],
        'unique_id': 11 * [1],
        'segment_unique_id': 11 * [0],
        'sample_unique_id': np.arange(0, 11),
        'unique_id_associated': 11 * [3],
        'sample': [[1.0, 1.0],
                   [1.2, 1.0],
                   [1.4, 1.0],
                   [1.6, 1.0],
                   [1.8, 1.0],
                   [2.0, 1.0],
                   [2.2, 1.0],
                   [2.4, 1.0],
                   [2.6, 1.0],
                   [2.8, 1.0],
                   [3.0, 1.0]],
        'projection_sample': [[1.0, 2.0],
                              [1.2, 2.0],
                              [1.4, 2.0],
                              [1.6, 2.0],
                              [1.8, 2.0],
                              [2.0, 2.0],
                              [2.2, 2.0],
                              [2.4, 2.0],
                              [2.6, 2.0],
                              [2.8, 2.0],
                              [3.0, 2.0]],
        'distance': 11 * [1.0],
        'BinaryClassification': ['FP', 'FP', 'FP', 'FP', 'FP', 'TP', 'TP', 'TP', 'TP', 'TP', 'TP'],
    })
    expected_estimate_evaluated['segment'] = sh.LineString([(1, 1), (3, 1)])
    expected_estimate_evaluated['segment_paired'] = sh.LineString([(2, 2), (4, 2)])
    return estimated_contours, reference_contours, expected_estimate_evaluated


@pytest.fixture()
def test_parallel2_setup():
    estimated_contours = pd.DataFrame(
        {
            'scan_index': [0, 0],
            'unique_id': [1, 1],
            'position_x': [1, 3],
            'position_y': [1, 1],
        })

    reference_contours = pd.DataFrame(
        {
            'scan_index': [0, 0],
            'unique_id': [3, 3],
            'position_x': [2, 4],
            'position_y': [2, 2],
        })
    expected_reference_evaluated = pd.DataFrame({
        'scan_index': 11 * [0],
        'unique_id': 11 * [3],
        'segment_unique_id': 11 * [0],
        'sample_unique_id': np.arange(0, 11),
        'unique_id_associated': 11 * [1],
        'sample': [[2.0, 2.0],
                   [2.2, 2.0],
                   [2.4, 2.0],
                   [2.6, 2.0],
                   [2.8, 2.0],
                   [3.0, 2.0],
                   [3.2, 2.0],
                   [3.4, 2.0],
                   [3.6, 2.0],
                   [3.8, 2.0],
                   [4.0, 2.0]],
        'projection_sample': [[2.0, 1.0],
                              [2.2, 1.0],
                              [2.4, 1.0],
                              [2.6, 1.0],
                              [2.8, 1.0],
                              [3.0, 1.0],
                              [3.2, 1.0],
                              [3.4, 1.0],
                              [3.6, 1.0],
                              [3.8, 1.0],
                              [4.0, 1.0]],
        'distance': 11 * [1.0],
        'BinaryClassification': ['TP', 'TP', 'TP', 'TP', 'TP', 'TP', 'FN', 'FN', 'FN', 'FN', 'FN'],
    })
    expected_reference_evaluated['segment'] = sh.LineString([(2, 2), (4, 2)])
    expected_reference_evaluated['segment_paired'] = sh.LineString([(1, 1), (3, 1)])
    return estimated_contours, reference_contours, expected_reference_evaluated


def test_parallel_segments(test_parallel_setup):
    estimated_contours, reference_contours, expected_estimate_evaluated = test_parallel_setup

    outputs = SGEvaluation(estimated_contours, reference_contours)
    outputs.SG_estimation_output.sort_values(['sample_unique_id'], inplace=True)
    outputs.SG_estimation_output.reset_index(inplace=True)
    outputs.SG_estimation_output.drop('index', inplace=True, axis=1)

    assert assert_frame_equal(outputs.SG_estimation_output, expected_estimate_evaluated,
                              check_like=True,
                              rtol=1.0e-2, atol=1.0e-2, check_dtype=False) is None


def test_parallel_segments2(test_parallel2_setup):
    estimated_contours, reference_contours, expected_reference_evaluated = test_parallel2_setup
    outputs = SGEvaluation(estimated_contours, reference_contours)
    outputs.SG_estimation_output.sort_values(['sample_unique_id'], inplace=True)

    outputs.SG_reference_output.reset_index(inplace=True)
    outputs.SG_reference_output.drop('index', inplace=True, axis=1)

    assert assert_frame_equal(outputs.SG_reference_output, expected_reference_evaluated, check_like=True,
                              rtol=1.0e-2, atol=1.0e-2,
                              check_dtype=False) is None


@pytest.fixture()
def test_two_setup():
    estimated_contours = pd.DataFrame(
        {
            'scan_index': [1, 1],
            'unique_id': [1, 1],
            'position_x': [2, 5],
            'position_y': [1, 1],
        })

    reference_contours = pd.DataFrame(
        {
            'scan_index': [1, 1],
            'unique_id': [2, 2],
            'position_x': [1, 4],
            'position_y': [1, 4],
        })
    expected_estimate_evaluated = pd.DataFrame({
        'scan_index': 16 * [1],
        'unique_id': 16 * [1],
        'segment_unique_id': 16 * [0],
        'sample_unique_id': np.arange(0, 16),
        'unique_id_associated': 16 * [2],
        'sample': [[2.0, 1.0],
                   [2.2, 1.0],
                   [2.4, 1.0],
                   [2.6, 1.0],
                   [2.8, 1.0],
                   [3.0, 1.0],
                   [3.2, 1.0],
                   [3.4, 1.0],
                   [3.6, 1.0],
                   [3.8, 1.0],
                   [4.0, 1.0],
                   [4.2, 1.0],
                   [4.4, 1.0],
                   [4.6, 1.0],
                   [4.8, 1.0],
                   [5.0, 1.0]],
        'projection_sample': [[2.0, 2.0],
                              [2.2, 2.2],
                              [2.4, 2.4],
                              [2.6, 2.6],
                              [2.8, 2.8],
                              [3.0, 3.0],
                              [3.2, 3.2],
                              [3.4, 3.4],
                              [3.6, 3.6],
                              [3.8, 3.8],
                              [4.0, 4.0],
                              [4.2, 4.2],
                              [4.4, 4.4],
                              [4.6, 4.6],
                              [4.8, 4.8],
                              [5.0, 5.0]],
        'distance': np.linspace(1.0, 4.0, 16),
        'BinaryClassification': ['TP', 'TP', 'TP', 'TP', 'TP', 'TP', 'FP', 'FP', 'FP', 'FP', 'FP', 'FP', 'FP', 'FP',
                                 'FP', 'FP'],
    })
    expected_estimate_evaluated['segment'] = sh.LineString([(2, 1), (5, 1)])
    expected_estimate_evaluated['segment_paired'] = sh.LineString([(1, 1), (4, 4)])
    return estimated_contours, reference_contours, expected_estimate_evaluated


def test_two_segments(test_two_setup):
    estimated_contours, reference_contours, expected_estimate_evaluated = test_two_setup
    outputs = SGEvaluation(estimated_contours, reference_contours)
    outputs.SG_estimation_output.sort_values(['sample_unique_id'], inplace=True)
    outputs.SG_estimation_output.reset_index(inplace=True)
    outputs.SG_estimation_output.drop('index', inplace=True, axis=1)

    assert assert_frame_equal(outputs.SG_estimation_output, expected_estimate_evaluated, check_like=True,
                              rtol=1.0e-2, atol=1.0e-2,
                              check_dtype=False) is None


if __name__ == '__main__':
    pytest.main()
