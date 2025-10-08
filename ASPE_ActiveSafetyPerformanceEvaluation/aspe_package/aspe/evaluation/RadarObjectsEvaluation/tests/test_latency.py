import pytest
from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures import Latency
import numpy as np
import pandas as pd
from itertools import product


@pytest.fixture()
def latency_object():
    thresholds = {'sine': {'min_max_spread': 0.01, 'diff_std_peak': 0.01, 'cost_fun_max': 100},
                  'parabola': {'min_max_spread': 0.01, 'diff_std_peak': 0.01, 'cost_fun_max': 100}}
    latency = Latency(thresholds=thresholds, min_delay=-0.5)
    return latency


def get_sine_df(t_min, t_max, dt, f, amplitude):
    t_vec = np.arange(t_min, t_max, dt)
    omega = 2 * np.pi * f
    sine = np.sin(omega * t_vec) * amplitude
    return pd.DataFrame({'timestamp': t_vec, 'sine': sine})


def get_parabola_df(t_min, t_max, dt):
    t_vec = np.arange(t_min, t_max, dt)
    parabola = np.square(t_vec)
    return pd.DataFrame({'timestamp': t_vec, 'parabola': parabola})


def add_noise_to_signal(df, signal_name, noise_spread):
    signal_len = len(df)
    noise = (np.random.rand(signal_len) - 0.5) * noise_spread
    df.loc[:, signal_name] += noise


def add_bias_to_signal(df, signal_name, bias_value):
    df.loc[:, signal_name] += bias_value


@pytest.mark.parametrize('latency_value', [0, -0.03, 0.03, -0.1, 0.1, 0.25, -0.25, -0.5, 0.5])
def test_sine_latency(latency_object, latency_value):
    sig_a = get_sine_df(t_min=0, t_max=5, dt=0.05, f=0.33, amplitude=5)
    sig_b = sig_a.copy()
    sig_b.loc[:, 'timestamp'] += latency_value
    estimated_latency = latency_object.calculate_latency_for_single_signal(sig_b, sig_a, 'sine')
    np.testing.assert_almost_equal(actual=estimated_latency, desired=latency_value, decimal=2)


@pytest.mark.parametrize('latency_value, noise_spread', product([0, -0.03, 0.03, -0.1, 0.1, 0.25, -0.25, -0.5, 0.5],
                         [0.01, 0.1, 0.25, 0.5]))
def test_sine_with_noise_latency(latency_object, latency_value, noise_spread):
    sig_a = get_sine_df(t_min=0, t_max=5, dt=0.05, f=0.33, amplitude=5)
    sig_b = sig_a.copy()
    sig_b.loc[:, 'timestamp'] += latency_value
    add_noise_to_signal(sig_b, 'sine', noise_spread)

    estimated_latency = latency_object.calculate_latency_for_single_signal(sig_b, sig_a, 'sine')
    np.testing.assert_almost_equal(actual=estimated_latency, desired=latency_value, decimal=2)


@pytest.mark.parametrize('latency_value, bias_value', product([0, -0.03, 0.03, -0.1, 0.1, 0.25, -0.25, -0.5, 0.5],
                                                              [-0.01, 0.01, -0.1, 0.1, -1, 1]))
def test_sine_with_bias_latency(latency_object, latency_value, bias_value):
    sig_a = get_sine_df(t_min=0, t_max=5, dt=0.05, f=0.33, amplitude=5)
    sig_b = sig_a.copy()
    sig_b.loc[:, 'timestamp'] += latency_value
    add_bias_to_signal(sig_b, 'sine', bias_value)

    estimated_latency = latency_object.calculate_latency_for_single_signal(sig_b, sig_a, 'sine')
    np.testing.assert_almost_equal(actual=estimated_latency, desired=latency_value, decimal=2)


@pytest.mark.parametrize('latency_value, bias_value, noise_spread', product([0, -0.03, 0.03, -0.1, 0.1],
                                                              [-0.01, 0.01, -0.1, 0.1, -1.0, 1.0],
                                                              [0.01, 0.1, 0.25]))
def test_sine_with_bias_and_noise_latency(latency_object, latency_value, bias_value, noise_spread):
    sig_a = get_sine_df(t_min=0, t_max=5, dt=0.05, f=0.33, amplitude=5)
    sig_b = sig_a.copy()
    sig_b.loc[:, 'timestamp'] += latency_value
    add_bias_to_signal(sig_b, 'sine', bias_value)
    add_noise_to_signal(sig_b, 'sine', noise_spread)

    estimated_latency = latency_object.calculate_latency_for_single_signal(sig_b, sig_a, 'sine')
    np.testing.assert_almost_equal(actual=estimated_latency, desired=latency_value, decimal=2)


@pytest.mark.parametrize('latency_value', [0, -0.03, 0.03, -0.1, 0.1, 0.25, -0.25, -0.5, 0.5])
def test_parabola_latency(latency_object, latency_value):
    sig_a = get_parabola_df(t_min=-2, t_max=3, dt=0.05)
    sig_b = sig_a.copy()
    sig_b.loc[:, 'timestamp'] += latency_value
    estimated_latency = latency_object.calculate_latency_for_single_signal(sig_b, sig_a, 'parabola')
    np.testing.assert_almost_equal(actual=estimated_latency, desired=latency_value, decimal=3)


@pytest.mark.parametrize('latency_value, noise_spread', product([0, -0.03, 0.03, -0.1, 0.1, 0.25, -0.25, -0.5, 0.5],
                         [0.01, 0.1, 0.25, 0.5]))
def test_parabola_with_noise_latency(latency_object, latency_value, noise_spread):
    sig_a = get_parabola_df(t_min=-1, t_max=4, dt=0.05)
    sig_b = sig_a.copy()
    sig_b.loc[:, 'timestamp'] += latency_value
    add_noise_to_signal(sig_b, 'parabola', noise_spread)

    estimated_latency = latency_object.calculate_latency_for_single_signal(sig_b, sig_a, 'parabola')
    np.testing.assert_almost_equal(actual=estimated_latency, desired=latency_value, decimal=2)


@pytest.mark.parametrize('latency_value, bias_value', product([0, -0.03, 0.03, -0.1, 0.1, 0.25, -0.25, -0.5, 0.5],
                                                              [-0.01, 0.01, -0.1, 0.1, -1, 1]))
def test_parabola_with_bias_latency(latency_object, latency_value, bias_value):
    sig_a = get_parabola_df(t_min=-1, t_max=4, dt=0.05)
    sig_b = sig_a.copy()
    sig_b.loc[:, 'timestamp'] += latency_value
    add_bias_to_signal(sig_b, 'parabola', bias_value)

    estimated_latency = latency_object.calculate_latency_for_single_signal(sig_b, sig_a, 'parabola')
    np.testing.assert_almost_equal(actual=estimated_latency, desired=latency_value, decimal=2)


@pytest.mark.parametrize('latency_value, bias_value, noise_spread', product([0, -0.03, 0.03, -0.1, 0.1],
                                                              [-0.01, 0.01, -0.1, 0.1, -1.0, 1.0],
                                                              [0.01, 0.1, 0.25]))
def test_parabola_with_bias_and_noise_latency(latency_object, latency_value, bias_value, noise_spread):
    sig_a = get_parabola_df(t_min=-1, t_max=4, dt=0.05)
    sig_b = sig_a.copy()
    sig_b.loc[:, 'timestamp'] += latency_value
    add_bias_to_signal(sig_b, 'parabola', bias_value)
    add_noise_to_signal(sig_b, 'parabola', noise_spread)

    estimated_latency = latency_object.calculate_latency_for_single_signal(sig_b, sig_a, 'parabola')
    np.testing.assert_almost_equal(actual=estimated_latency, desired=latency_value, decimal=2)