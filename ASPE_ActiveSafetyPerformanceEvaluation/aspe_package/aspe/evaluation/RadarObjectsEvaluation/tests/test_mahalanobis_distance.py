import numpy as np
import pandas as pd
import pytest

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.distance_metrics.MahalanobisDistance import \
    MahalanobisDistance


def assert_with_threshold(v1, v2, thres=0.):
    assert abs(v1 - v2) <= thres


@pytest.mark.parametrize(
    'pos_x_value_a, pos_y_value_a, pos_x_value_b, pos_y_value_b, expected_value',
    [
        [0, 0, 0, 1, 1],
        [0, 0, 1, 0, 1],
        [0, 1, 0, 0, 1],
        [1, 0, 0, 0, 1],
        [0, 0, 0, 2, 2],
        [0, 0, 3, 0, 3],
        [0, 4, 0, 0, 4],
        [5, 0, 0, 0, 5],
    ],
)
def test_mahalanobis_distance_scale_1(pos_x_value_a, pos_y_value_a, pos_x_value_b, pos_y_value_b, expected_value):
    m_dist = MahalanobisDistance()
    data_a_source = {
        m_dist.pos_x_signal_name: pos_x_value_a,
        m_dist.pos_y_signal_name: pos_y_value_a,
        m_dist.orientation_signal_name: 0,
    }
    data_a = pd.DataFrame(data_a_source, index=[0])
    data_b_source = {
        m_dist.pos_x_signal_name: pos_x_value_b,
        m_dist.pos_y_signal_name: pos_y_value_b,
    }
    data_b = pd.DataFrame(data_b_source, index=[0])
    dist = m_dist.calculate(data_a, data_b).loc[0, 'mahalanobis_distance_2d']
    assert_with_threshold(dist, expected_value)


@pytest.mark.parametrize(
    'orientation_a, expected_value',
    [[p, 1] for p in np.arange(-np.pi, np.pi, 0.1)],
)
def test_mahalanobis_distance_scale_1_with_rotation(orientation_a, expected_value):
    m_dist = MahalanobisDistance()
    data_a_source = {
        m_dist.pos_x_signal_name: 0,
        m_dist.pos_y_signal_name: 0,
        m_dist.orientation_signal_name: orientation_a,
    }
    data_a = pd.DataFrame(data_a_source, index=[0])
    data_b_source = {
        m_dist.pos_x_signal_name: 1,
        m_dist.pos_y_signal_name: 0,
    }
    data_b = pd.DataFrame(data_b_source, index=[0])
    dist = m_dist.calculate(data_a, data_b).loc[0, 'mahalanobis_distance_2d']
    assert_with_threshold(dist, expected_value, 1e-3)


@pytest.mark.parametrize(
    'pos_x_value_a, pos_y_value_a, pos_x_value_b, pos_y_value_b, scale_y, expected_value',
    [
        [0, 0, 0, 1, 2, 2],
        [0, 0, 1, 0, 2, 1],
        [0, 1, 0, 0, 2, 2],
        [1, 0, 0, 0, 2, 1],
        [0, 0, 0, 2, 2, 4],
        [0, 0, 3, 0, 2, 3],
        [0, 4, 0, 0, 2, 8],
        [5, 0, 0, 0, 2, 5],
        [0, 0, 0, 1, 3, 3],
        [0, 0, 1, 0, 3, 1],
        [0, 1, 0, 0, 3, 3],
        [1, 0, 0, 0, 3, 1],
        [0, 0, 0, 2, 3, 6],
        [0, 0, 3, 0, 3, 3],
        [0, 4, 0, 0, 3, 12],
        [5, 0, 0, 0, 3, 5],
    ],
)
def test_mahalanobis_distance_scale_x_not_1(scale_y,
                                            pos_x_value_a,
                                            pos_y_value_a,
                                            pos_x_value_b,
                                            pos_y_value_b,
                                            expected_value):
    m_dist = MahalanobisDistance(scale_y=scale_y)
    data_a_source = {
        m_dist.pos_x_signal_name: pos_x_value_a,
        m_dist.pos_y_signal_name: pos_y_value_a,
        m_dist.orientation_signal_name: 0,
    }
    data_a = pd.DataFrame(data_a_source, index=[0])
    data_b_source = {
        m_dist.pos_x_signal_name: pos_x_value_b,
        m_dist.pos_y_signal_name: pos_y_value_b,
    }
    data_b = pd.DataFrame(data_b_source, index=[0])
    dist = m_dist.calculate(data_a, data_b).loc[0, 'mahalanobis_distance_2d']
    assert_with_threshold(dist, expected_value)


@pytest.mark.parametrize(
    'pos_x_value_a, pos_y_value_a, pos_x_value_b, pos_y_value_b, scale_x, expected_value',
    [
        [0, 0, 0, 1, 2, 1],
        [0, 0, 1, 0, 2, 2],
        [0, 1, 0, 0, 2, 1],
        [1, 0, 0, 0, 2, 2],
        [0, 0, 0, 2, 2, 2],
        [0, 0, 3, 0, 2, 6],
        [0, 4, 0, 0, 2, 4],
        [5, 0, 0, 0, 2, 10],
        [0, 0, 0, 1, 3, 1],
        [0, 0, 1, 0, 3, 3],
        [0, 1, 0, 0, 3, 1],
        [1, 0, 0, 0, 3, 3],
        [0, 0, 0, 2, 3, 2],
        [0, 0, 3, 0, 3, 9],
        [0, 4, 0, 0, 3, 4],
        [5, 0, 0, 0, 3, 15],
    ],
)
@pytest.mark.parametrize(
    'pos_x_value_a, pos_y_value_a, pos_x_value_b, pos_y_value_b, scale_x, scale_y, expected_value',
    [
        [0, 0, 2, 1, 2, 3, np.sqrt(25)],
        [0, 1, 1, 0, 2, 4, np.sqrt(20)],
        [0, 0, -2, 2, 2, 5, np.sqrt(116)],
        [0, 0, 1, -1, 2, 6, np.sqrt(40)],
    ],
)
def test_mahalanobis_distance_scale_x_and_y_not_1(scale_x,
                                                  scale_y,
                                                  pos_x_value_a,
                                                  pos_y_value_a,
                                                  pos_x_value_b,
                                                  pos_y_value_b,
                                                  expected_value):
    m_dist = MahalanobisDistance(scale_x=scale_x, scale_y=scale_y)
    data_a_source = {
        m_dist.pos_x_signal_name: pos_x_value_a,
        m_dist.pos_y_signal_name: pos_y_value_a,
        m_dist.orientation_signal_name: 0,
    }
    data_a = pd.DataFrame(data_a_source, index=[0])
    data_b_source = {
        m_dist.pos_x_signal_name: pos_x_value_b,
        m_dist.pos_y_signal_name: pos_y_value_b,
    }
    data_b = pd.DataFrame(data_b_source, index=[0])
    dist = m_dist.calculate(data_a, data_b).loc[0, 'mahalanobis_distance_2d']
    assert_with_threshold(dist, expected_value)


@pytest.mark.parametrize(
    'orientation_data_a, expected_value',
    [
        [0, 1],
        [np.pi / 2, 2],
        [-np.pi / 2, 2],
        [np.pi, 1],
        [-np.pi, 1],
        [np.pi / 4, np.sqrt(5 / 2)],
    ],
)
def test_mahalanobis_distance_scale_y_not_1_with_rotation(orientation_data_a, expected_value):
    m_dist = MahalanobisDistance(scale_y=2)
    data_a_source = {
        m_dist.pos_x_signal_name: 0,
        m_dist.pos_y_signal_name: 0,
        m_dist.orientation_signal_name: orientation_data_a,
    }
    data_a = pd.DataFrame(data_a_source, index=[0])
    data_b_source = {
        m_dist.pos_x_signal_name: 1,
        m_dist.pos_y_signal_name: 0,
    }
    data_b = pd.DataFrame(data_b_source, index=[0])
    dist = m_dist.calculate(data_a, data_b).loc[0, 'mahalanobis_distance_2d']
    assert_with_threshold(dist, expected_value, 1e-3)
