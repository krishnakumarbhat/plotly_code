import pandas as pd
import pytest
from pandas.testing import assert_series_equal

from aspe.evaluation.RadarSGEvaluation.DataStructures.SGSamples import SGSamples
from aspe.evaluation.RadarSGEvaluation.Enums.SegmentationType import SegmentationType
from aspe.evaluation.RadarSGEvaluation.SegmentationType.specify_segmentation_type import specify_segmentation_type


@pytest.fixture()
def samples_est():
    return SGSamples()


@pytest.fixture()
def samples_ref():
    return SGSamples()


def test_all_true(samples_est, samples_ref):
    samples_ref.signals['multiple_segmentation'] = [True for _ in range(0, 4)]
    samples_ref.signals['classification'] = [True for _ in range(0, 4)]

    samples_est.signals['multiple_segmentation'] = [True for _ in range(0, 4)]
    samples_est.signals['classification'] = [True for _ in range(0, 4)]

    result_ref = specify_segmentation_type(samples_ref.signals, SegmentationType.OverSegmentation)
    result_est = specify_segmentation_type(samples_est.signals, SegmentationType.UnderSegmentation)

    exp_ref = pd.Series(name='segmentation_type', data=[SegmentationType.OverSegmentation for _ in range(0, 4)])
    exp_est = pd.Series(name='segmentation_type', data=[SegmentationType.UnderSegmentation for _ in range(0, 4)])

    assert assert_series_equal(result_ref, exp_ref) is None
    assert assert_series_equal(result_est, exp_est) is None


def test_all_false(samples_est, samples_ref):
    samples_ref.signals['multiple_segmentation'] = [False for _ in range(0, 4)]
    samples_ref.signals['classification'] = [True for _ in range(0, 4)]

    samples_est.signals['multiple_segmentation'] = [False for _ in range(0, 4)]
    samples_est.signals['classification'] = [True for _ in range(0, 4)]

    result_ref = specify_segmentation_type(samples_ref.signals, SegmentationType.OverSegmentation)
    result_est = specify_segmentation_type(samples_est.signals, SegmentationType.UnderSegmentation)

    exp = pd.Series(name='segmentation_type', data=[SegmentationType.NotAssigned for _ in range(0, 4)])

    assert assert_series_equal(result_ref, exp) is None
    assert assert_series_equal(result_est, exp) is None


def test_mixed(samples_est, samples_ref):
    samples_ref.signals['multiple_segmentation'] = [False, True, True, False]
    samples_ref.signals['classification'] = [True for _ in range(0, 4)]

    samples_est.signals['multiple_segmentation'] = [True, True, True, False]
    samples_est.signals['classification'] = [True for _ in range(0, 4)]

    result_ref = specify_segmentation_type(samples_ref.signals, SegmentationType.OverSegmentation)
    result_est = specify_segmentation_type(samples_est.signals, SegmentationType.UnderSegmentation)

    exp_ref = pd.Series(name='segmentation_type',
                        data=[SegmentationType.NotAssigned,
                              SegmentationType.OverSegmentation,
                              SegmentationType.OverSegmentation,
                              SegmentationType.NotAssigned])

    exp_est = pd.Series(name='segmentation_type',
                        data=[SegmentationType.UnderSegmentation,
                              SegmentationType.UnderSegmentation,
                              SegmentationType.UnderSegmentation,
                              SegmentationType.NotAssigned])

    assert assert_series_equal(result_ref, exp_ref) is None
    assert assert_series_equal(result_est, exp_est) is None


def test_mixed_2(samples_est, samples_ref):
    samples_ref.signals['multiple_segmentation'] = [False, True, True, False]
    samples_ref.signals['classification'] = [True for _ in range(0, 4)]

    samples_est.signals['multiple_segmentation'] = [False, False, False, False]
    samples_est.signals['classification'] = [True for _ in range(0, 4)]

    result_ref = specify_segmentation_type(samples_ref.signals, SegmentationType.OverSegmentation)
    result_est = specify_segmentation_type(samples_est.signals, SegmentationType.UnderSegmentation)

    exp_ref = pd.Series(name='segmentation_type',
                        data=[SegmentationType.NotAssigned,
                              SegmentationType.OverSegmentation,
                              SegmentationType.OverSegmentation,
                              SegmentationType.NotAssigned])

    exp_est = pd.Series(name='segmentation_type',
                        data=[SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned,
                              SegmentationType.NotAssigned])

    assert assert_series_equal(result_ref, exp_ref) is None
    assert assert_series_equal(result_est, exp_est) is None
