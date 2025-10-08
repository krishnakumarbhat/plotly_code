"""
Unittests for SGPipeline.
"""
import numpy as np
import pandas as pd
import pytest
from pandas.testing import assert_frame_equal

from aspe.evaluation.RadarSGEvaluation.SGPEPipeline import SGPEPipeline
from aspe.extractors.ENV.DataSets.ENVExtractedData import ENVExtractedData
from aspe.extractors.ENV.DataSets.ENVStationaryGeometries import ENVStationaryGeometries


@pytest.fixture()
def test_same_segments_setup():
    sg_df = pd.DataFrame({'scan_index': [0, 0, 0, 2, 2, 4, 4, 4, 4],
                          'timestamp': [0.1, 0.1, 0.1, 0.2, 0.2, 0.4, 0.4, 0.4, 0.4],
                          'unique_id': [1, 1, 1, 5, 5, 7, 7, 7, 7],
                          'position_x': [0.0, 1.0, 1.0, -3.0, -4.0, 1.5, 1.0, 3.0, 4.0],
                          'position_y': [1.0, 3.0, 4.0, -2.0, -4.0, -1.5, -1.0, -3.0, -2.0],
                          })
    extracted_data = ENVExtractedData()
    extracted_data.stationary_geometries = ENVStationaryGeometries()
    extracted_data.stationary_geometries.signals = sg_df

    return extracted_data


def test_SGPipeline_same_segments_1(test_same_segments_setup):
    extracted_data = test_same_segments_setup

    pipeline = SGPEPipeline()
    pipeline.evaluate(extracted_data, extracted_data)

    assert pipeline.kpi_output.general_results['F1_score'].values == 1.0
    assert pipeline.kpi_output.general_results['PPV'].values == 1.0
    assert pipeline.kpi_output.general_results['FNs'].values == 0
    assert pipeline.kpi_output.general_results['FPs'].values == 0
    assert pipeline.kpi_output.general_results['TPR'].values == 1.0
    assert pipeline.kpi_output.general_results['estimated_TPs'].values == pipeline.kpi_output.general_results[
        'reference_TPs'].values


def test_SGPipeline_same_segments_2(test_same_segments_setup):
    extracted_data = test_same_segments_setup

    expected_sample_results = pd.DataFrame({'q25_distance': [np.float32(0.0)],
                                            'q50_distance': [np.float32(0.0)],
                                            'q75_distance': [np.float32(0.0)],
                                            'q99_distance': [np.float32(0.0)],
                                            'bias_deviation_x': [np.float32(0.0)],
                                            'mean_abs_deviation_x': [np.float32(0.0)],
                                            'std_deviation_x': [np.float32(0.0)],
                                            'q99_deviation_x': [np.float32(0.0)],
                                            'bias_deviation_y': [np.float32(0.0)],
                                            'mean_abs_deviation_y': [np.float32(0.0)],
                                            'std_deviation_y': [np.float32(0.0)],
                                            'q99_deviation_y': [np.float32(0.0)],
                                            })

    pipeline = SGPEPipeline()
    pipeline.evaluate(extracted_data, extracted_data)

    assert assert_frame_equal(pipeline.kpi_output.estimated_results, expected_sample_results) is None
    assert assert_frame_equal(pipeline.kpi_output.reference_results, expected_sample_results) is None


@pytest.fixture()
def test_extended_setup():
    estimated_sg_df = pd.DataFrame(
        {
            'scan_index': [1, 1, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6],
            'timestamp': [1, 1, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6],
            'unique_id': [1, 1, 5, 5, 2, 2, 2, 7, 7, 7, 1, 1, 3, 3, 7, 7, 1, 1],
            'position_x': [2, 7, 2, 7, 2, 2, 7, -1, -2, -2, -5, -4, -2, 0, 1, 6, 1, 1],
            'position_y': [1, 1, 1, 1, 5, 1, 1, 7, 6, -2, 1, 4, -2, 0, 2, 2, 2, 3]
        })
    reference_sg_df = pd.DataFrame(
        {
            'scan_index': [1, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5],
            'timestamp': [1, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 5],
            'unique_id': [2, 2, 2, 3, 3, 3, 3, 5, 5, 1, 1, 1, 1, 1, 3, 3, 3, 3],
            'position_x': [3, 6, 8, 3, 6, 0, 1, 4, 8, -2, -1, -1, -3, 0, 1, 4, 4, 6],
            'position_y': [4, 2, 2, 2, 0, 5, 1, -1, 0, -2, 1, 7, -1, -1, 0, 2, 3, 3]
        })

    estimated_data = ENVExtractedData()
    reference_data = ENVExtractedData()
    estimated_data.stationary_geometries = ENVStationaryGeometries()
    reference_data.stationary_geometries = ENVStationaryGeometries()
    estimated_data.stationary_geometries.signals = estimated_sg_df
    reference_data.stationary_geometries.signals = reference_sg_df

    return estimated_data, reference_data


def test_SGPipeline_extended_general_results(test_extended_setup):
    estimated_data, reference_data = test_extended_setup

    expected_kpi_general_results = pd.DataFrame({
        'F1_score': [np.float32(0.74403)],
        'FPs': [np.int32(26)],
        'PPV': [np.float32(0.71739)],
        'undersegmentation_ratio': [np.float32(0.054347824)],
        'estimated_TPs': [np.int32(66)],
        'FNs': [np.int32(20)],
        'TPR': [np.float32(0.77272)],
        'oversegmentation_ratio': [np.float32(0.056818184)],
        'reference_TPs': [np.int32(68)]
    })

    pipeline = SGPEPipeline()
    pipeline.evaluate(estimated_data, reference_data)

    assert assert_frame_equal(pipeline.kpi_output.general_results, expected_kpi_general_results) is None


def test_SGPipeline_extended_estimated_results(test_extended_setup):
    estimated_data, reference_data = test_extended_setup

    expected_kpi_estimated_results = pd.DataFrame({'q25_distance': [np.float32(0.66667)],
                                                   'q50_distance': [np.float32(1.0)],
                                                   'q75_distance': [np.float32(1.375)],
                                                   'q99_distance': [np.float32(2.0)],
                                                   'bias_deviation_x': [np.float32(-0.005050505)],
                                                   'mean_abs_deviation_x': [np.float32(0.47475)],
                                                   'std_deviation_x': [np.float32(0.74809)],
                                                   'q99_deviation_x': [np.float32(1.91875)],
                                                   'bias_deviation_y': [np.float32(0.1016414)],
                                                   'mean_abs_deviation_y': [np.float32(0.58649)],
                                                   'std_deviation_y': [np.float32(0.87247)],
                                                   'q99_deviation_y': [np.float32(2.0)],
                                                   })

    pipeline = SGPEPipeline()
    pipeline.evaluate(estimated_data, reference_data)

    assert assert_frame_equal(pipeline.kpi_output.estimated_results, expected_kpi_estimated_results) is None


def test_SGPipeline_extended_reference_results(test_extended_setup):
    estimated_data, reference_data = test_extended_setup

    expected_kpi_reference_results = pd.DataFrame({'q25_distance': [np.float32(0.60199)],
                                                   'q50_distance': [np.float32(1.0)],
                                                   'q75_distance': [np.float32(1.21635)],
                                                   'q99_distance': [np.float32(1.94702)],
                                                   'bias_deviation_x': [np.float32(0.15032683)],
                                                   'mean_abs_deviation_x': [np.float32(0.58170)],
                                                   'std_deviation_x': [np.float32(0.75247)],
                                                   'q99_deviation_x': [np.float32(1.81444)],
                                                   'bias_deviation_y': [np.float32(-0.10988561)],
                                                   'mean_abs_deviation_y': [np.float32(0.57312)],
                                                   'std_deviation_y': [np.float32(0.77175)],
                                                   'q99_deviation_y': [np.float32(1.81444)],
                                                   })

    pipeline = SGPEPipeline()
    pipeline.evaluate(estimated_data, reference_data)

    assert assert_frame_equal(pipeline.kpi_output.reference_results, expected_kpi_reference_results) is None


@pytest.fixture()
def test_small_setup():
    estimated_sg_df = pd.DataFrame(
        {
            'scan_index': [1, 1, 1],
            'timestamp': [1, 1, 1],
            'unique_id': [1, 1, 1],
            'position_x': [2.0, 2.0, 3.0],
            'position_y': [1.0, 5.0, 5.0]
        })
    reference_sg_df = pd.DataFrame(
        {
            'scan_index': [1, 1],
            'timestamp': [1, 1],
            'unique_id': [2, 2],
            'position_x': [1.0, 1.0],
            'position_y': [1.0, 3.0]
        })

    estimated_data = ENVExtractedData()
    reference_data = ENVExtractedData()
    estimated_data.stationary_geometries = ENVStationaryGeometries()
    reference_data.stationary_geometries = ENVStationaryGeometries()
    estimated_data.stationary_geometries.signals = estimated_sg_df
    reference_data.stationary_geometries.signals = reference_sg_df

    return estimated_data, reference_data


def test_SGPipeline_small_general_results(test_small_setup):
    estimated_data, reference_data = test_small_setup

    expected_kpi_general_results = pd.DataFrame({
        'F1_score': [np.float32(0.58824)],
        'FPs': [np.int32(7)],
        'PPV': [np.float32(0.41667)],
        'undersegmentation_ratio': [np.float32(0.0)],
        'estimated_TPs': [np.int32(5)],
        'FNs': [np.int32(0)],
        'TPR': [np.float32(1.0)],
        'oversegmentation_ratio': [np.float32(0.0)],
        'reference_TPs': [np.int32(5)]
    })

    pipeline = SGPEPipeline()
    pipeline.evaluate(estimated_data, reference_data)

    assert assert_frame_equal(pipeline.kpi_output.general_results, expected_kpi_general_results) is None


def test_SGPipeline_small_estimated_results(test_small_setup):
    estimated_data, reference_data = test_small_setup

    expected_kpi_estimated_results = pd.DataFrame({'q25_distance': [np.float32(1.0)],
                                                   'q50_distance': [np.float32(1.0)],
                                                   'q75_distance': [np.float32(1.0)],
                                                   'q99_distance': [np.float32(1.0)],
                                                   'bias_deviation_x': [np.float32(1.0)],
                                                   'mean_abs_deviation_x': [np.float32(1.0)],
                                                   'std_deviation_x': [np.float32(0.0)],
                                                   'q99_deviation_x': [np.float32(1.0)],
                                                   'bias_deviation_y': [np.float32(0.0)],
                                                   'mean_abs_deviation_y': [np.float32(0.0)],
                                                   'std_deviation_y': [np.float32(0.0)],
                                                   'q99_deviation_y': [np.float32(0.0)],
                                                   })

    pipeline = SGPEPipeline()
    pipeline.evaluate(estimated_data, reference_data)

    assert assert_frame_equal(pipeline.kpi_output.estimated_results, expected_kpi_estimated_results) is None


def test_SGPipeline_small_reference_results(test_small_setup):
    estimated_data, reference_data = test_small_setup

    expected_kpi_reference_results = pd.DataFrame({'q25_distance': [np.float32(1.0)],
                                                   'q50_distance': [np.float32(1.0)],
                                                   'q75_distance': [np.float32(1.0)],
                                                   'q99_distance': [np.float32(1.0)],
                                                   'bias_deviation_x': [np.float32(-1.0)],
                                                   'mean_abs_deviation_x': [np.float32(1.0)],
                                                   'std_deviation_x': [np.float32(0.0)],
                                                   'q99_deviation_x': [np.float32(1.0)],
                                                   'bias_deviation_y': [np.float32(0.0)],
                                                   'mean_abs_deviation_y': [np.float32(0.0)],
                                                   'std_deviation_y': [np.float32(0.0)],
                                                   'q99_deviation_y': [np.float32(0.0)],
                                                   })

    pipeline = SGPEPipeline()
    pipeline.evaluate(estimated_data, reference_data)

    assert assert_frame_equal(pipeline.kpi_output.reference_results, expected_kpi_reference_results) is None
