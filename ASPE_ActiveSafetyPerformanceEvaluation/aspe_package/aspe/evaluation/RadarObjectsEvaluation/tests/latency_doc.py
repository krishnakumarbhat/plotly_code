from aspe.evaluation.RadarObjectsEvaluation.tests.test_latency import get_parabola_df, add_bias_to_signal, \
    add_noise_to_signal
import numpy as np
import pytest
from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures import Latency
import numpy as np



thresholds = {'sine': {'min_max_spread': 0.01, 'diff_std_peak': 0.01, 'rms': 100},
              'parabola': {'min_max_spread': 0.01, 'diff_std_peak': 0.01, 'rms': 100}}
latency = Latency(thresholds=thresholds, max_delay=1)

sig_a = get_parabola_df(t_min=-1, t_max=10, dt=0.05)
sig_b = sig_a.copy()

latency_value = 0.5
bias_value = 3
noise_spread = 0.05
sig_b.loc[:, 'timestamp'] += latency_value
add_bias_to_signal(sig_b, 'parabola', bias_value)
add_noise_to_signal(sig_b, 'parabola', noise_spread)

estimated_latency = latency.calculate_latency_for_single_signal(sig_b, sig_a, 'parabola')
