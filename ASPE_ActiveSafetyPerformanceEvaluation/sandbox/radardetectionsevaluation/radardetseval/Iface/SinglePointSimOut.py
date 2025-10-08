import pandas as pd
import numpy as np


class SinglePointSimOut:

    def __init__(self):
        signals = [
            'sim_ref_x',
            'sim_ref_y',
            'sim_ref_vx',
            'sim_ref_vy',
            'sim_ref_range',
            'sim_ref_azimuth',
            'sim_ref_range_rate',
            'sim_ref_cross_radial_vel',
            'sim_meas_range',
            'sim_meas_azimuth',
            'sim_meas_range_rate'
        ]
        self.signals = pd.DataFrame(columns=signals)
        self.coordinate_system = 'SCS'
        self.gt_x = float('nan')
        self.gt_y = float('nan')
        self.gt_vx = float('nan')
        self.gt_vy = float('nan')
        self.gt_range = float('nan')
        self.gt_azimuth = float('nan')
        self.gt_range_rate = float('nan')
        self.gt_cross_radial_vel = float('nan')
        self.ref_cov = np.zeros([4, 4])
        self.meas_cov = np.zeros([3, 3])
