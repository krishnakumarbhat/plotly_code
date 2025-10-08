import numpy as np


class DefaultRadar:
    version = 1
    description = 'Just a guess'
    range_std = 0.2
    azimuth_std = np.deg2rad(0.3)
    range_rate_std = 0.04
