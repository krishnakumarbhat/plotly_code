# coding=utf-8
"""
Unittests for SGEvaluationDraftMII
"""
import pytest

import numpy as np
import pandas as pd

from pandas.testing import assert_frame_equal
from aspe.evaluation.RadarSGEvaluation.SGEvaluationDraftMII import SGEvaluation


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
        'sample_unique_id': np.arange(0, 11),
        'unique_id_associated': 11 * [3],
        'sample': [np.array([1.0, 1.0]),
                   np.array([1.2, 1.0]),
                   np.array([1.4, 1.0]),
                   np.array([1.6, 1.0]),
                   np.array([1.8, 1.0]),
                   np.array([2.0, 1.0]),
                   np.array([2.2, 1.0]),
                   np.array([2.4, 1.0]),
                   np.array([2.6, 1.0]),
                   np.array([2.8, 1.0]),
                   np.array([3.0, 1.0])],
        'distance': [np.nan,
                     np.nan,
                     np.nan,
                     np.nan,
                     np.nan,
                     1.0,
                     1.0,
                     1.0,
                     1.0,
                     1.0,
                     1.0],
        'BinaryClassification': ['FP', 'FP', 'FP', 'FP', 'FP', 'TP', 'TP', 'TP', 'TP', 'TP', 'TP'],
    })
    return estimated_contours, reference_contours, expected_estimate_evaluated


def test_parallel_contours(test_parallel_setup):
    estimated_contours, reference_contours, expected_estimate_evaluated = test_parallel_setup
    output = SGEvaluation(estimated_contours, reference_contours)

    assert assert_frame_equal(output.SG_estimation, expected_estimate_evaluated, check_like=True,
                              rtol=1.0e-2, atol=1.0e-2,
                              check_dtype=False) is None


@pytest.fixture()
def test_parallel_2_setup():
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
        'sample_unique_id': np.arange(0, 11),
        'unique_id_associated': 11 * [1],
        'sample': [np.array([2.0, 2.0]),
                   np.array([2.2, 2.0]),
                   np.array([2.4, 2.0]),
                   np.array([2.6, 2.0]),
                   np.array([2.8, 2.0]),
                   np.array([3.0, 2.0]),
                   np.array([3.2, 2.0]),
                   np.array([3.4, 2.0]),
                   np.array([3.6, 2.0]),
                   np.array([3.8, 2.0]),
                   np.array([4.0, 2.0])],
        'distance': [1.0,
                     1.0,
                     1.0,
                     1.0,
                     1.0,
                     1.0,
                     np.nan,
                     np.nan,
                     np.nan,
                     np.nan,
                     np.nan],
        'BinaryClassification': ['TP', 'TP', 'TP', 'TP', 'TP', 'TP', 'FN', 'FN', 'FN', 'FN', 'FN'],
    })
    return estimated_contours, reference_contours, expected_reference_evaluated


def test_parallel_contours_2(test_parallel_2_setup):
    estimated_contours, reference_contours, expected_reference_evaluated = test_parallel_2_setup
    output = SGEvaluation(estimated_contours, reference_contours)

    assert assert_frame_equal(output.SG_reference, expected_reference_evaluated, check_like=True,
                              rtol=1.0e-2, atol=1.0e-2,
                              check_dtype=False) is None


@pytest.fixture()
def test_two_segments_setup():
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
        'sample_unique_id': np.arange(0, 16),
        'unique_id_associated': 16 * [2],
        'sample': [np.array([2.0, 1.0]),
                   np.array([2.2, 1.0]),
                   np.array([2.4, 1.0]),
                   np.array([2.6, 1.0]),
                   np.array([2.8, 1.0]),
                   np.array([3.0, 1.0]),
                   np.array([3.2, 1.0]),
                   np.array([3.4, 1.0]),
                   np.array([3.6, 1.0]),
                   np.array([3.8, 1.0]),
                   np.array([4.0, 1.0]),
                   np.array([4.2, 1.0]),
                   np.array([4.4, 1.0]),
                   np.array([4.6, 1.0]),
                   np.array([4.8, 1.0]),
                   np.array([5.0, 1.0])],
        'distance': [0.7071,
                     0.8485,
                     0.9899,
                     1.1313,
                     1.2727,
                     1.4142,
                     1.5556,
                     1.6970,
                     1.8384,
                     1.9798,
                     np.nan,
                     np.nan,
                     np.nan,
                     np.nan,
                     np.nan,
                     np.nan],
        'BinaryClassification': ['TP', 'TP', 'TP', 'TP', 'TP', 'TP', 'TP', 'TP', 'TP', 'TP', 'FP', 'FP', 'FP', 'FP',
                                 'FP', 'FP'],
    })
    return estimated_contours, reference_contours, expected_estimate_evaluated


def test_two_segments(test_two_segments_setup):
    estimated_contours, reference_contours, expected_estimate_evaluated = test_two_segments_setup
    output = SGEvaluation(estimated_contours, reference_contours)

    assert assert_frame_equal(output.SG_estimation, expected_estimate_evaluated, check_like=True,
                              rtol=1.0e-2, atol=1.0e-2,
                              check_dtype=False) is None


if __name__ == '__main__':
    pytest.main()
