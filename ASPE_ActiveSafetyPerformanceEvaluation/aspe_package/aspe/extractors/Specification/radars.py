import numpy as np


class DefaultRadar:
    version = 1
    description = 'Just a guess'
    range_std = 0.1
    azimuth_std = np.deg2rad(0.3)
    elevation_std = np.deg2rad(2.0)
    range_rate_std = 0.04
