import pandas as pd
import pytest
from pandas.testing import assert_series_equal

from aspe.evaluation.RadarSGEvaluation.BinaryClassification.specify_bc_type import specify_bc_type
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSamples import SGSamples
from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType


@pytest.fixture()
def samples_est():
    return SGSamples()


@pytest.fixture()
def samples_ref():
    return SGSamples()


def test_all_true(samples_est, samples_ref):
    samples_ref.signals['classification'] = [True for _ in range(0, 4)]
    samples_est.signals['classification'] = [True for _ in range(0, 4)]

    result_ref = specify_bc_type(samples_ref.signals, BCType.FalseNegative)
    result_est = specify_bc_type(samples_est.signals, BCType.FalsePositive)

    exp = pd.Series(name='binary_classification', data=[BCType.TruePositive for _ in range(0, 4)])

    assert assert_series_equal(result_ref, exp) is None
    assert assert_series_equal(result_est, exp) is None


def test_all_false(samples_est, samples_ref):
    samples_ref.signals['classification'] = [False for _ in range(0, 4)]
    samples_est.signals['classification'] = [False for _ in range(0, 4)]

    result_ref = specify_bc_type(samples_ref.signals, BCType.FalseNegative)
    result_est = specify_bc_type(samples_est.signals, BCType.FalsePositive)

    exp_est = pd.Series(name='binary_classification', data=[BCType.FalsePositive for _ in range(0, 4)])
    exp_ref = pd.Series(name='binary_classification', data=[BCType.FalseNegative for _ in range(0, 4)])

    assert assert_series_equal(result_ref, exp_ref) is None
    assert assert_series_equal(result_est, exp_est) is None


def test_mixed_1(samples_est, samples_ref):
    samples_ref.signals['classification'] = [False, True, True, True]
    samples_est.signals['classification'] = [False, True, True, False]

    result_ref = specify_bc_type(samples_ref.signals, BCType.FalseNegative)
    result_est = specify_bc_type(samples_est.signals, BCType.FalsePositive)

    exp_ref = pd.Series(name='binary_classification',
                        data=[BCType.FalseNegative,
                              BCType.TruePositive,
                              BCType.TruePositive,
                              BCType.TruePositive])

    exp_est = pd.Series(name='binary_classification',
                        data=[BCType.FalsePositive,
                              BCType.TruePositive,
                              BCType.TruePositive,
                              BCType.FalsePositive,
                              ])

    assert assert_series_equal(result_ref, exp_ref) is None
    assert assert_series_equal(result_est, exp_est) is None


def test_mixed_2(samples_est, samples_ref):
    samples_ref.signals['classification'] = [False, False, True, True]
    samples_est.signals['classification'] = [False, True, False, True]

    result_ref = specify_bc_type(samples_ref.signals, BCType.FalseNegative)
    result_est = specify_bc_type(samples_est.signals, BCType.FalsePositive)

    exp_ref = pd.Series(name='binary_classification',
                        data=[BCType.FalseNegative,
                              BCType.FalseNegative,
                              BCType.TruePositive,
                              BCType.TruePositive])

    exp_est = pd.Series(name='binary_classification',
                        data=[BCType.FalsePositive,
                              BCType.TruePositive,
                              BCType.FalsePositive,
                              BCType.TruePositive])

    assert assert_series_equal(result_ref, exp_ref) is None
    assert assert_series_equal(result_est, exp_est) is None


def test_empty(samples_est, samples_ref):
    result_ref = specify_bc_type(samples_ref.signals, BCType.FalseNegative)
    result_est = specify_bc_type(samples_est.signals, BCType.FalsePositive)

    exp = pd.Series(name='binary_classification', dtype='object')

    assert assert_series_equal(result_ref, exp) is None
    assert assert_series_equal(result_est, exp) is None

