import numpy as np
import pytest

from aspe.evaluation.RadarSGEvaluation.DataStructures.SGKPIOutput import SGKPIOutput


@pytest.fixture()
def test_init_setup():
    results = {'estimated_TPs': 0,
               'reference_TPs': 0,
               'FPs': 0,
               'FNs': 0,
               'TPR': 0,
               'PPV': 0,
               'F1_score': 0,
               'oversegmentation_ratio': 0,
               'undersegmentation_ratio': 0}

    sample_results = {'bias_deviation_x': 0,
                      'mean_abs_deviation_x': 0,
                      'std_deviation_x': 0,
                      'q99_deviation_x': 0,
                      'bias_deviation_y': 0,
                      'mean_abs_deviation_y': 0,
                      'std_deviation_y': 0,
                      'q99_deviation_y': 0,
                      'q25_distance': 0,
                      'q50_distance': 0,
                      'q75_distance': 0,
                      'q99_distance': 0}

    expected_segment_results_dict_keys = ['bias_deviation_x', 'mean_abs_deviation_x', 'std_deviation_x',
                                          'q99_deviation_x',
                                          'bias_deviation_y', 'mean_abs_deviation_y', 'std_deviation_y',
                                          'q99_deviation_y',
                                          'q25_distance', 'q50_distance', 'q75_distance', 'q99_distance']
    expected_general_results_dict_keys = ['estimated_TPs', 'reference_TPs', 'FPs', 'FNs', 'TPR', 'PPV', 'F1_score',
                                          'oversegmentation_ratio', 'undersegmentation_ratio']  # noqa
    return results, sample_results, expected_segment_results_dict_keys, expected_general_results_dict_keys


def test_SGKPIOutput_init(test_init_setup):
    results, sample_results, expected_segment_results_dict_keys, expected_general_results_dict_keys = test_init_setup

    kpi = SGKPIOutput(results, sample_results, sample_results)

    assert list(kpi.estimated_results.columns) == expected_segment_results_dict_keys
    assert list(kpi.reference_results.columns) == expected_segment_results_dict_keys
    assert list(kpi.general_results.columns) == expected_general_results_dict_keys


@pytest.fixture()
def test_setup_1():
    results = {'estimated_TPs': 40,
               'reference_TPs': 50,
               'FPs': 2,
               'FNs': 4,
               'TPR': 0.9,
               'PPV': 0.8,
               'F1_score': 0.1,
               'oversegmentation_ratio': 0.1,
               'undersegmentation_ratio': 0.1}

    est_sample_results = {'bias_deviation_x': 0.1,
                          'mean_abs_deviation_x': 0.1,
                          'std_deviation_x': 0,
                          'q99_deviation_x': 0,
                          'bias_deviation_y': 0,
                          'mean_abs_deviation_y': 0,
                          'std_deviation_y': 0,
                          'q99_deviation_y': 0,
                          'q25_distance': 0,
                          'q50_distance': 0,
                          'q75_distance': 0.2,
                          'q99_distance': 0}
    ref_sample_results = {'bias_deviation_x': 0.2,
                          'mean_abs_deviation_x': 0.5,
                          'std_deviation_x': 0,
                          'q99_deviation_x': 0,
                          'bias_deviation_y': 0,
                          'mean_abs_deviation_y': 0,
                          'std_deviation_y': 0,
                          'q99_deviation_y': 0,
                          'q25_distance': 1,
                          'q50_distance': 0,
                          'q75_distance': 0,
                          'q99_distance': 0}

    return results, est_sample_results, ref_sample_results


def test_SGKPIOutput_1(test_setup_1):
    results, est_sample_results, ref_sample_results = test_setup_1
    kpi = SGKPIOutput(results, est_sample_results, ref_sample_results)

    assert kpi.general_results.estimated_TPs.values == 40
    assert kpi.general_results.estimated_TPs.dtype == np.int32
    assert kpi.estimated_results.bias_deviation_x.values == 0.1
    assert kpi.estimated_results.bias_deviation_x.dtype == np.float32
    assert kpi.reference_results.bias_deviation_x.values == 0.2
    assert kpi.reference_results.bias_deviation_x.dtype == np.float32
