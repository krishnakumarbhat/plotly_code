import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag


class IsPerpendicularToHost(IFlag):
    """ Class for configurable flag calculation: check if object is moving perpendicular to host """

    def __init__(self, expected_abs_bbox_orientation=np.deg2rad(90),
                 max_allowed_abs_bbox_orientation_difference=np.deg2rad(45), flag_signature='is_perpendicular_to_host',
                 *args, **kwargs):
        """
        Setting initial parameters
        :param expected_abs_bbox_orientation: expected absolute value of bbox_orientation
        :param max_allowed_abs_bbox_orientation_difference: max allowed absolute difference between
            expected and actual bbox_orientation
        :param kwargs:
        """
        super().__init__(flag_signature, *args, **kwargs)
        self.expected_abs_bbox_orientation = expected_abs_bbox_orientation
        self.max_allowed_abs_bbox_orientation_difference = max_allowed_abs_bbox_orientation_difference

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        obj_orientation_difference = abs(
            data_frame.bounding_box_orientation) - self.expected_abs_bbox_orientation
        is_moving_perpendicular_to_host = abs(
            obj_orientation_difference) <= self.max_allowed_abs_bbox_orientation_difference
        output = pd.Series(is_moving_perpendicular_to_host, index=data_frame.index)
        return output
