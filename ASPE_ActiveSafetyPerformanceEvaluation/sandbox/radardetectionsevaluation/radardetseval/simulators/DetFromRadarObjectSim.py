import numpy as np
import pandas as pd
from scipy.stats import uniform

from radardetseval.simulators.sim_det_from_point import sim_single_det_from_scs_cart_point
from AptivDataExtractors.utilities.MathFunctions import calc_velocity_in_position, calc_position_in_bounding_box


class DetFromRadarObjectSim:
    """
    Simulator of detection based on object
    """

    def __init__(self, gt_radar_object: pd.Series):
        """

        :param gt_radar_object:
        """
        self.gt_radar_object = gt_radar_object

    def sim_single_point_from_ref_point(self, meas_cov_matrix: np.array,
                                        bounding_box_refpoint_long_offset_ratio,
                                        bounding_box_refpoint_lat_offset_ratio,
                                        meas_bias: np.array = np.zeros(3)):
        """

        :param meas_cov_matrix:
        :param bounding_box_refpoint_long_offset_ratio:
        :param bounding_box_refpoint_lat_offset_ratio:
        :param meas_bias:
        :return:
        """
        pos_x, pos_y = calc_position_in_bounding_box(self.gt_radar_object.position_x,
                                                     self.gt_radar_object.position_y,
                                                     self.gt_radar_object.bounding_box_dimensions_x,
                                                     self.gt_radar_object.bounding_box_dimensions_y,
                                                     self.gt_radar_object.bounding_box_orientation,
                                                     self.gt_radar_object.bounding_box_refpoint_long_offset_ratio,
                                                     self.gt_radar_object.bounding_box_refpoint_lat_offset_ratio,
                                                     bounding_box_refpoint_long_offset_ratio,
                                                     bounding_box_refpoint_lat_offset_ratio)

        vel_x, vel_y = calc_velocity_in_position(self.gt_radar_object.position_x,
                                                 self.gt_radar_object.position_y,
                                                 self.gt_radar_object.velocity_otg_x,
                                                 self.gt_radar_object.velocity_otg_y,
                                                 self.gt_radar_object.yaw_rate,
                                                 pos_x, pos_y)

        gt_state = np.array([pos_x, pos_y, vel_x, vel_y])

        detection = sim_single_det_from_scs_cart_point(gt_state, meas_cov_matrix, meas_bias)
        return detection

    def sim_single_point_randomly(self, meas_cov_matrix: np.array,
                                        meas_bias: np.array = np.zeros(3)):
        """

        :param meas_cov_matrix:
        :param meas_bias:
        :return:
        """

        ref_points_draw = uniform.rvs(size=2)  # two ref points in range 0,1
        output = self.sim_single_point_from_ref_point(meas_cov_matrix, ref_points_draw[0], ref_points_draw[1],
                                                      meas_bias)
        return output, ref_points_draw

    def sim_single_point_randomly_from_area(self, meas_cov_matrix: np.array,
                                            ref_point_x_min, ref_point_x_max, ref_point_y_min, ref_point_y_max,
                                            meas_bias: np.array = np.zeros(3)):
        """

        :param meas_cov_matrix:
        :param ref_point_x_min:
        :param ref_point_x_max:
        :param ref_point_y_min:
        :param ref_point_y_max:
        :param meas_bias:
        :return:
        """

        ref_points_draw = uniform.rvs(size=2)  # two ref points in range 0,1
        # scaling
        ref_points_draw[0] = ref_points_draw[0] * (ref_point_x_max - ref_point_x_min) + ref_point_x_min
        ref_points_draw[1] = ref_points_draw[1] * (ref_point_y_max - ref_point_y_min) + ref_point_y_min
        output = self.sim_single_point_from_ref_point(meas_cov_matrix, ref_points_draw[0], ref_points_draw[1],
                                                      meas_bias)
        return output, ref_points_draw

    def sim_single_point_randomly_from_line(self, meas_cov_matrix: np.array,
                                            ref_point_x_1, ref_point_x_2, ref_point_y_1, ref_point_y_2,
                                            meas_bias: np.array = np.zeros(3)):
        """

        :param meas_cov_matrix:
        :param ref_point_x_1:
        :param ref_point_x_2:
        :param ref_point_y_1:
        :param ref_point_y_2:
        :param meas_bias:
        :return:
        """

        ref_points_id_draw = uniform.rvs(size=1)  # two ref points in range 0,1
        xp = np.array([0.0, 1.0])
        ref_points_x = np.interp(ref_points_id_draw, xp, np.array([ref_point_x_1, ref_point_x_2]))
        ref_points_y = np.interp(ref_points_id_draw, xp, np.array([ref_point_y_1, ref_point_y_2]))
        ref_points_draw = np.array([ref_points_x[0], ref_points_y[0]])
        output = self.sim_single_point_from_ref_point(meas_cov_matrix, ref_points_draw[0], ref_points_draw[1],
                                                      meas_bias)
        return output, ref_points_draw


