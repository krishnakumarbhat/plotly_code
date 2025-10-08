import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy.optimize import curve_fit

from aspe.utilities.MathFunctions import normalize_angle_vector


def handle_cos_negative_amplitude(amplitude: float, phase: float):
    if amplitude < 0:
        amplitude = -amplitude
        phase = phase - np.pi
    return amplitude, phase


def calculate_velocity_profile(dets: pd.DataFrame):
    def form(azimuth, amplitude, phase):
        return amplitude * np.cos(-azimuth + phase)

    azimuth, rrate = dets.azimuth_vcs.to_numpy(), dets.range_rate_comp.to_numpy()
    (speed, orientation), covariance = curve_fit(form, azimuth, rrate,
                                                 p0=np.array([5, 0]),
                                                 bounds=(np.array([0, -np.pi]), np.array([50, np.pi])),
                                                 )
    return speed, orientation, covariance