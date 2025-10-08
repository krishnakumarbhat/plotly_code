from copy import deepcopy
import numpy as np
import pandas as pd
from scipy.stats import multivariate_normal


class RadarObjectSim:
    """
    Simulator of object
    """
    def __init__(self, gt_radar_object: pd.Series):
        """

        :param gt_radar_object:
        """
        self.gt_radar_object = gt_radar_object

    def sim(self, pos_cov_matrix: np.array, rel_vel_cov_matrix: np.array,
            yaw_rate_var=0.0):

        deviation_pos = multivariate_normal.rvs(mean=np.zeros(2),
                                                cov=pos_cov_matrix,
                                                size=1)
        deviation_vel = multivariate_normal.rvs(mean=np.zeros(2),
                                                cov=rel_vel_cov_matrix,
                                                size=1)
        deviation_yaw_rate = multivariate_normal.rvs(mean=np.zeros(1),
                                                     cov=yaw_rate_var,
                                                     size=1)

        output = deepcopy(self.gt_radar_object)
        output['position_x'] += deviation_pos[0]
        output['position_y'] += deviation_pos[1]
        output['center_x'] += deviation_pos[0]
        output['center_y'] += deviation_pos[1]
        output['position_variance_x'] = pos_cov_matrix[0, 0]
        output['position_variance_y'] = pos_cov_matrix[1, 1]
        output['position_covariance'] = pos_cov_matrix[1, 0]
        output['velocity_otg_x'] += deviation_vel[0]
        output['velocity_otg_y'] += deviation_vel[1]
        output['velocity_otg_variance_x'] = rel_vel_cov_matrix[0, 0]
        output['velocity_otg_variance_y'] = rel_vel_cov_matrix[1, 1]
        output['velocity_otg_covariance'] = rel_vel_cov_matrix[1, 0]
        output['yaw_rate'] += deviation_yaw_rate

        return output
