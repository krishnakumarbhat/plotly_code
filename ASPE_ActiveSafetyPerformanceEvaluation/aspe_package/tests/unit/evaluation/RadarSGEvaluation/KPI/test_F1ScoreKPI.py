"""
Unittest for F1Score
"""
import numpy as np
import pandas as pd
import pytest

from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType
from aspe.evaluation.RadarSGEvaluation.KPI.F1ScoreKPI import F1ScoreKPI


def test_F1ScoreKPI_value():
    estimated_bc_data = [BCType.FalsePositive, BCType.TruePositive, BCType.FalsePositive, BCType.FalsePositive,
                         BCType.TruePositive, BCType.TruePositive, BCType.FalsePositive, BCType.FalsePositive]
    reference_bc_data = [BCType.TruePositive, BCType.TruePositive, BCType.FalseNegative, BCType.TruePositive,
                         BCType.TruePositive, BCType.TruePositive, BCType.FalseNegative, BCType.FalseNegative]
    estimated_df = pd.DataFrame({'binary_classification': estimated_bc_data})
    reference_df = pd.DataFrame({'binary_classification': reference_bc_data})
    f1score = F1ScoreKPI()
    output = f1score.calculate(estimated_df['binary_classification'], reference_df['binary_classification'])
    expected_output = 0.46875
    assert output == expected_output
    assert isinstance(output, np.float32)


def test_F1ScoreKPI_zero_values():
    estimated_bc_data = [BCType.FalsePositive] * 8
    reference_bc_data = [BCType.TruePositive, BCType.TruePositive, BCType.FalseNegative, BCType.TruePositive,
                         BCType.TruePositive, BCType.TruePositive, BCType.FalseNegative, BCType.FalseNegative]
    estimated_df = pd.DataFrame({'binary_classification': estimated_bc_data})
    reference_df = pd.DataFrame({'binary_classification': reference_bc_data})
    f1score = F1ScoreKPI()
    output = f1score.calculate(estimated_df['binary_classification'], reference_df['binary_classification'])
    expected_output = 0.0
    assert output == expected_output
    assert isinstance(output, np.float32)


def test_F1ScoreKPI_value_one():
    estimated_bc_data = [BCType.TruePositive] * 13
    reference_bc_data = [BCType.TruePositive] * 13
    estimated_df = pd.DataFrame({'binary_classification': estimated_bc_data})
    reference_df = pd.DataFrame({'binary_classification': reference_bc_data})
    f1score = F1ScoreKPI()
    output = f1score.calculate(estimated_df['binary_classification'], reference_df['binary_classification'])
    expected_output = 1.0
    assert output == expected_output
    assert isinstance(output, np.float32)


def test_F1ScoreKPI_empty_estimated_data():
    estimated_df = pd.DataFrame({'binary_classification': []})
    reference_df = pd.DataFrame({'binary_classification': [1, 3, 3, 1, 1, 3]})
    f1score = F1ScoreKPI()
    with pytest.raises(ValueError, match='No data in at least one of series.'):
        f1score.calculate(estimated_df['binary_classification'],
                          reference_df['binary_classification'])


def test_F1ScoreKPI_empty_reference_data():
    estimated_df = pd.DataFrame({'binary_classification': [1, 2, 2, 1, 1, 2]})
    reference_df = pd.DataFrame({'binary_classification': []})
    f1score = F1ScoreKPI()
    with pytest.raises(ValueError, match='No data in at least one of series.'):
        f1score.calculate(estimated_df['binary_classification'],
                          reference_df['binary_classification'])


def test__F1ScoreKPI_exception_estimated():
    estimated_bc_data = [1, 0, -1, 2, 0, 1, 3, 2, -1, 0]
    reference_bc_data = [BCType.TruePositive, BCType.TruePositive, BCType.FalseNegative, BCType.TruePositive,
                         BCType.TruePositive, BCType.TruePositive, BCType.FalseNegative, BCType.FalseNegative]
    estimated_df = pd.DataFrame({'binary_classification': estimated_bc_data})
    reference_df = pd.DataFrame({'binary_classification': reference_bc_data})
    f1score = F1ScoreKPI()
    with pytest.raises(ValueError, match='Binary_classification column with BCType was expected for estimated data.'):
        f1score.calculate(estimated_df['binary_classification'],
                          reference_df['binary_classification'])


def test_F1ScoreKPI_exception_reference():
    estimated_bc_data = [BCType.FalsePositive, BCType.TruePositive, BCType.FalsePositive, BCType.FalsePositive,
                         BCType.TruePositive, BCType.TruePositive, BCType.FalsePositive, BCType.FalsePositive]
    reference_bc_data = [1, 3, -2, 1, 0, 3, 2, 1, -1, 0]
    estimated_df = pd.DataFrame({'binary_classification': estimated_bc_data})
    reference_df = pd.DataFrame({'binary_classification': reference_bc_data})
    f1score = F1ScoreKPI()
    with pytest.raises(ValueError, match='Binary_classification column with BCType was expected for reference data.'):
        f1score.calculate(estimated_df['binary_classification'],
                          reference_df['binary_classification'])
