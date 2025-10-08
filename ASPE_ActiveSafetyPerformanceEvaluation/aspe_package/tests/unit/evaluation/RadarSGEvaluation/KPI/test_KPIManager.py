"""
Unittest for KPIManager
"""
import numpy as np
import pytest
import pandas as pd
from pandas.testing import assert_frame_equal
from aspe.evaluation.RadarSGEvaluation.DataStructures import EvaluationSignals as EvSig
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSampleFeatures import SGSampleFeatures
from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType
from aspe.evaluation.RadarSGEvaluation.Enums.SegmentationType import SegmentationType
from aspe.evaluation.RadarSGEvaluation.KPI.KPIManager import calculate_kpis


@pytest.fixture()
def test_setup_1():
    sample_features_est_df = pd.DataFrame({
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
    sample_features_est = SGSampleFeatures()
    sample_features_est.insert_feature(sample_features_est_df)

    sample_features_ref_df = pd.DataFrame({
        'scan_index': [0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 4, 4],
        'timestamp': [0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.4, 0.4],
        'unique_id': [0, 2, 4, 6, 8, 11, 12, 15, 17, 18, 19, 20, 21, 22, 23, 24, 25],
        'distance': 17 * [0.0],
        'deviation_x': 17 * [0.0],
        'deviation_y': 17 * [0.0],
        'binary_classification': 17 * [BCType.TruePositive],
        'segmentation_type': [SegmentationType.OverSegmentation, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.OverSegmentation, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.OverSegmentation],
    })
    sample_features_ref = SGSampleFeatures()
    sample_features_ref.insert_feature(sample_features_ref_df)

    expected_general_output = pd.DataFrame({'estimated_TPs': [np.int32(18)],
                                            'reference_TPs': [np.int32(17)],
                                            'FPs': [np.int32(0)],
                                            'FNs': [np.int32(0)],
                                            'TPR': [np.float32(1.0)],
                                            'PPV': [np.float32(1.0)],
                                            'F1_score': [np.float32(1.0)],
                                            'oversegmentation_ratio': [np.float32(0.17647058823529413)],
                                            'undersegmentation_ratio': [np.float32(0.1111111111111111)]})

    return sample_features_est, sample_features_ref, expected_general_output


def test_KPIManager_1(test_setup_1):
    sample_features_est, sample_features_ref, expected_general_output = test_setup_1
    kpi_output = calculate_kpis(sample_features_est.signals, sample_features_ref.signals)

    assert assert_frame_equal(kpi_output.general_results, expected_general_output, check_like=True) is None


@pytest.fixture()
def test_setup_for_small_dataset():
    sample_features_est_df = pd.DataFrame({
        'scan_index': [0, 0, 0, 0, 0, 0],
        'timestamp': [0.2, 0.2, 0.2, 0.4, 0.4, 0.4],
        'unique_id': [0, 1, 2, 4, 6, 8],
        'distance': [0.1, 10.0, 0.3, 0.1, 0.3, 0.2],
        'deviation_x': [0.1, 10.0, -0.1, -0.1, -0.2, -0.2],
        'deviation_y': [-0.1, 0.0, 0.1, 0.1, 0.2, 0.2],
        'binary_classification': [BCType.TruePositive, BCType.FalsePositive, BCType.TruePositive, BCType.TruePositive,
                                  BCType.TruePositive, BCType.TruePositive],
        'segmentation_type': [SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.UnderSegmentation, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned],
    })
    sample_features_est = SGSampleFeatures()
    sample_features_est.insert_feature(sample_features_est_df)

    sample_features_ref_df = pd.DataFrame({
        'scan_index': [0, 0, 0, 0, 0, 0],
        'timestamp': [0.2, 0.2, 0.2, 0.4, 0.4, 0.4],
        'unique_id': [0, 1, 2, 4, 6, 8],
        'distance': [0.1, 10.0, 0.3, 0.1, 0.3, 0.2],
        'deviation_x': [0.1, 10.0, -0.1, -0.1, -0.2, -0.2],
        'deviation_y': [-0.1, 0.0, 0.1, 0.1, 0.2, 0.2],
        'binary_classification': [BCType.TruePositive, BCType.FalseNegative, BCType.TruePositive, BCType.TruePositive,
                                  BCType.TruePositive, BCType.TruePositive],
        'segmentation_type': [SegmentationType.OverSegmentation, SegmentationType.OverSegmentation,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned, SegmentationType.NotAssigned],
    })
    sample_features_ref = SGSampleFeatures()
    sample_features_ref.insert_feature(sample_features_ref_df)

    expected_estimated_results = pd.DataFrame({'bias_deviation_x': [-0.1],
                                               'mean_abs_deviation_x': [0.14],
                                               'std_deviation_x': [0.10954451],
                                               'q99_deviation_x': [0.2],
                                               'bias_deviation_y': [0.1],
                                               'mean_abs_deviation_y': [0.14],
                                               'std_deviation_y': [0.10954451],
                                               'q99_deviation_y': [0.2],
                                               'q25_distance': [0.1],
                                               'q50_distance': [0.2],
                                               'q75_distance': [0.3],
                                               'q99_distance': [0.3]}).astype(np.float32)

    expected_reference_results = pd.DataFrame({'bias_deviation_x': [-0.1],
                                               'mean_abs_deviation_x': [0.14],
                                               'std_deviation_x': [0.10954451],
                                               'q99_deviation_x': [0.2],
                                               'bias_deviation_y': [0.1],
                                               'mean_abs_deviation_y': [0.14],
                                               'std_deviation_y': [0.10954451],
                                               'q99_deviation_y': [0.2],
                                               'q25_distance': [0.1],
                                               'q50_distance': [0.2],
                                               'q75_distance': [0.3],
                                               'q99_distance': [0.3]}).astype(np.float32)
    return sample_features_est, sample_features_ref, expected_estimated_results, expected_reference_results


def test_KPIManager_small_dataset(test_setup_for_small_dataset):
    sample_features_est, sample_features_ref, expected_estimated_results, expected_reference_results = test_setup_for_small_dataset
    kpi_output = calculate_kpis(sample_features_est.signals, sample_features_ref.signals)

    assert assert_frame_equal(kpi_output.estimated_results, expected_estimated_results, check_like=True) is None
    assert assert_frame_equal(kpi_output.reference_results, expected_reference_results, check_like=True) is None
