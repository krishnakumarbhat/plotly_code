"""
Unittests for KPI series
"""
import numpy as np
import pandas as pd
import pytest

from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType
from aspe.evaluation.RadarSGEvaluation.Enums.SegmentationType import SegmentationType
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.BiasKPI import BiasKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.FalseNegativeKPI import FalseNegativeKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.FalsePositiveKPI import FalsePositiveKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.MeanAbsKPI import MeanAbsKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.OverSegmentationRatioKPI import OverSegmentationRatioKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.PredictivePositiveValueKPI import PPVCalculationKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.QuantileKPI import QuantileKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.RmsKPI import RmsKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.StdKPI import StdKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.TruePositiveKPI import TruePositiveKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.TruePositiveRatioKPI import TPRCalculationKPI
from aspe.evaluation.RadarSGEvaluation.KPI.SeriesKPI.UnderSegmentationRatioKPI import UnderSegmentationRatioKPI


@pytest.fixture()
def test_setup_1():
    v_x = [-1.0, -0.9, -1.0, -1.1, -1.0, -0.8, -1.2, -1.0, -0.9, -0.7]
    v_y = [1.0, 0.9, 1.0, 1.1, 1.0, 0.8, 1.2, 1.0, 0.9, 0.7]
    v_distance = [1.4, 1.3, 1.4, 1.6, 1.4, 1.2, 1.8, 1.4, 1.3, 1.0]
    v_binary_classification = 10 * [BCType.TruePositive]
    v_segmentation = 10 * [SegmentationType.UnderSegmentation]
    error_threshold = 0.01

    output = pd.DataFrame({'deviation_x': v_x,
                           'deviation_y': v_y,
                           'distance': v_distance,
                           'binary_classification': v_binary_classification,
                           'segmentation_type': v_segmentation,
                           })
    return output, error_threshold


def test_BiasKPI_1(test_setup_1):
    df, error_threshold = test_setup_1
    series = BiasKPI()
    output = series.calculate(df['deviation_x'])
    assert output == pytest.approx(-0.96, error_threshold)
    assert isinstance(output, np.float32)


def test_MeanAbsKPI_1(test_setup_1):
    df, error_threshold = test_setup_1
    series = MeanAbsKPI()
    output = series.calculate(df['deviation_x'])
    assert output == pytest.approx(0.96, error_threshold)
    assert isinstance(output, np.float32)


def test_RmsKPI_1(test_setup_1):
    df, error_threshold = test_setup_1
    series = RmsKPI()
    output = series.calculate(df['deviation_x'])
    assert output == pytest.approx(0.97, error_threshold)
    assert isinstance(output, np.float32)


def test_StdKPI_1(test_setup_1):
    df, error_threshold = test_setup_1
    series = StdKPI()
    output = series.calculate(df['deviation_y'])
    assert output == pytest.approx(0.136, error_threshold)
    assert isinstance(output, np.float32)


def test_QuantileKPI_1(test_setup_1):
    df, error_threshold = test_setup_1
    series = QuantileKPI(99)
    output = series.calculate(df['distance'])
    assert output == pytest.approx(1.78, error_threshold)
    assert isinstance(output, np.float32)


def test_TruePositiveKPI_1(test_setup_1):
    df, _ = test_setup_1
    series = TruePositiveKPI()
    output = series.calculate(df['binary_classification'])
    expected_output = 10
    assert output == expected_output
    assert isinstance(output, np.int64)


def test_FalseNegativeKPI_1(test_setup_1):
    df, _ = test_setup_1
    series = FalseNegativeKPI()
    output = series.calculate(df['binary_classification'])
    assert output == 0
    assert isinstance(output, np.int64)


def test_FalsePositiveKPI_1(test_setup_1):
    df, _ = test_setup_1
    series = FalsePositiveKPI()
    output = series.calculate(df['binary_classification'])
    assert output == 0
    assert isinstance(output, np.int64)


def test_OverSegmentationRatioKPI_1(test_setup_1):
    df, error_threshold = test_setup_1
    series = OverSegmentationRatioKPI()
    output = series.calculate(df['segmentation_type'])
    assert output == pytest.approx(0.0, error_threshold)
    assert isinstance(output, np.float32)


def test_UnderSegmentationRatioKPI_1(test_setup_1):
    df, error_threshold = test_setup_1
    series = UnderSegmentationRatioKPI()
    output = series.calculate(df['segmentation_type'])
    assert output == pytest.approx(1.0, error_threshold)
    assert isinstance(output, np.float32)


def test_TruePositiveRatio_1(test_setup_1):
    df, error_threshold = test_setup_1
    series = TPRCalculationKPI()
    output = series.calculate(df['binary_classification'])
    assert output == pytest.approx(1.0, error_threshold)
    assert isinstance(output, np.float32)


def test_PredictivePositiveValue_1(test_setup_1):
    df, error_threshold = test_setup_1
    series = PPVCalculationKPI()
    output = series.calculate(df['binary_classification'])
    assert output == pytest.approx(1.0, error_threshold)
    assert isinstance(output, np.float32)


@pytest.fixture()
def test_setup_2():
    v_x = [0.1, np.nan, -0.1, -0.2, np.nan]
    v_y = [-0.1, np.nan, 0.1, 0.2, np.nan]
    v_distance = [0.14, np.nan, 0.14, 0.29, np.nan]
    v_binary_classification = [BCType.TruePositive, BCType.FalsePositive, BCType.TruePositive, BCType.TruePositive,
                               BCType.FalsePositive]
    v_segmentation = [SegmentationType.OverSegmentation, SegmentationType.NotAssigned, SegmentationType.NotAssigned,
                      SegmentationType.UnderSegmentation, SegmentationType.NotAssigned]
    error_threshold = 0.01
    output = pd.DataFrame({'deviation_x': v_x,
                           'deviation_y': v_y,
                           'distance': v_distance,
                           'binary_classification': v_binary_classification,
                           'segmentation_type': v_segmentation,
                           })
    return output, error_threshold


def test_BiasKPI_2(test_setup_2):
    df, error_threshold = test_setup_2
    series = BiasKPI()
    output = series.calculate(df['deviation_y'])
    assert output == pytest.approx(0.067, error_threshold)
    assert isinstance(output, np.float32)


def test_MeanAbsKPI_2(test_setup_2):
    df, error_threshold = test_setup_2
    series = MeanAbsKPI()
    output = series.calculate(df['deviation_y'])
    assert output == pytest.approx(0.133, error_threshold)
    assert isinstance(output, np.float32)


def test_RmsKPI_2(test_setup_2):
    df, error_threshold = test_setup_2
    series = RmsKPI()
    output = series.calculate(df['deviation_x'])
    assert output == pytest.approx(0.141, error_threshold)
    assert isinstance(output, np.float32)


def test_StdKPI_2(test_setup_2):
    df, error_threshold = test_setup_2
    series = StdKPI()
    output = series.calculate(df['deviation_x'])
    assert output == pytest.approx(0.124, error_threshold)
    assert isinstance(output, np.float32)


def test_QuantileKPI_2(test_setup_2):
    df, error_threshold = test_setup_2
    series = QuantileKPI(50)
    output = series.calculate(df['deviation_y'])
    assert output == pytest.approx(0.1, error_threshold)
    assert isinstance(output, np.float32)


def test_TruePositiveKPI_2(test_setup_2):
    df, _ = test_setup_2
    series = TruePositiveKPI()
    output = series.calculate(df['binary_classification'])
    expected_output = 3
    assert output == expected_output
    assert isinstance(output, np.int64)


def test_FalseNegativeKPI_2(test_setup_2):
    df, _ = test_setup_2
    series = FalseNegativeKPI()
    output = series.calculate(df['binary_classification'])
    assert output == 0
    assert isinstance(output, np.int64)


def test_FalsePositiveKPI_2(test_setup_2):
    df, _ = test_setup_2
    series = FalsePositiveKPI()
    output = series.calculate(df['binary_classification'])
    assert output == 2
    assert isinstance(output, np.int64)


def test_OverSegmentationRatio_2(test_setup_2):
    df, error_threshold = test_setup_2
    series = OverSegmentationRatioKPI()
    output = series.calculate(df['segmentation_type'])
    assert output == pytest.approx(0.2, error_threshold)
    assert isinstance(output, np.float32)


def test_UnderSegmentationRatioKPI_2(test_setup_2):
    df, error_threshold = test_setup_2
    series = UnderSegmentationRatioKPI()
    output = series.calculate(df['segmentation_type'])
    assert output == pytest.approx(0.2, error_threshold)
    assert isinstance(output, np.float32)


def test_TruePositiveRatioKPI_2(test_setup_2):
    df, error_threshold = test_setup_2
    series = TPRCalculationKPI()
    output = series.calculate(df['binary_classification'])
    assert output == pytest.approx(1.0, error_threshold)
    assert isinstance(output, np.float32)


def test_PredictivePositiveValueKPI_2(test_setup_2):
    df, error_threshold = test_setup_2
    series = PPVCalculationKPI()
    output = series.calculate(df['binary_classification'])
    assert output == pytest.approx(0.6, error_threshold)
    assert isinstance(output, np.float32)


@pytest.fixture()
def test_exceptions_setup():
    v_x = [2.0, -1.0, np.nan]
    v_y = [-1.0, -2.0, np.nan]
    v_distance = [8.0, 9.0, np.nan]
    v_binary_classification = 3 * [-1]
    v_segmentation = 3 * [np.nan]
    error_threshold = 0.01
    output = pd.DataFrame({'deviation_x': v_x,
                           'deviation_y': v_y,
                           'distance': v_distance,
                           'binary_classification': v_binary_classification,
                           'segmentation_type': v_segmentation,
                           })
    return output, error_threshold


def test_BiasKPI_exception(test_exceptions_setup):
    df, error_threshold = test_exceptions_setup
    series = BiasKPI()
    output = series.calculate(df['deviation_x'])
    assert output == pytest.approx(0.5, error_threshold)


def test_MeanAbsKPI_exception(test_exceptions_setup):
    df, error_threshold = test_exceptions_setup
    series = MeanAbsKPI()
    output = series.calculate(df['deviation_y'])
    assert output == pytest.approx(1.5, error_threshold)


def test_QuantileKPI_exception():
    with pytest.raises(ValueError, match='Expected value from 0:100 interval. ') as exit_info:
        QuantileKPI(-1)


def test_TruePositiveKPI_exception(test_exceptions_setup):
    df, _ = test_exceptions_setup
    series = TruePositiveKPI()
    with pytest.raises(ValueError, match='binary_classification columns with BCType was expected. ') as exit_info:
        series.calculate(df['binary_classification'])


def test_FalseNegativeKPI_exception(test_exceptions_setup):
    df, _ = test_exceptions_setup
    series = FalseNegativeKPI()
    with pytest.raises(ValueError, match='binary_classification column with BCType values was expected. ') as exit_info:
        series.calculate(df['binary_classification'])


def test_FalsePositiveKPI_exception(test_exceptions_setup):
    df, _ = test_exceptions_setup
    series = FalsePositiveKPI()
    with pytest.raises(ValueError, match='binary_classification column with BCType values was expected. ') as exit_info:
        series.calculate(df['binary_classification'])


def test_OverSegmentationRatioKPI_exception(test_exceptions_setup):
    df, _ = test_exceptions_setup
    series = OverSegmentationRatioKPI()
    with pytest.raises(ValueError,
                       match='multiple_segmentation column with SegmentationType values was expected. ') as exit_info:
        series.calculate(df['segmentation_type'])


def test_UnderSegmentationRatioKPI_exception(test_exceptions_setup):
    df, _ = test_exceptions_setup
    series = UnderSegmentationRatioKPI()
    with pytest.raises(ValueError,
                       match='multiple_segmentation column with SegmentationType values was expected. ') as exit_info:
        series.calculate(df['segmentation_type'])


@pytest.fixture()
def test_empty_setup():
    return pd.DataFrame({'deviation_x': [],
                         'deviation_y': [],
                         'distance': [],
                         'binary_classification': [],
                         'segmentation_type': [],
                         })


def test_UndereSegmentationRatioKPI_empty_series(test_empty_setup):
    df = test_empty_setup
    series = UnderSegmentationRatioKPI()
    with pytest.raises(ValueError,
                       match='No data in series') as exit_info:
        series.calculate(df['segmentation_type'])
