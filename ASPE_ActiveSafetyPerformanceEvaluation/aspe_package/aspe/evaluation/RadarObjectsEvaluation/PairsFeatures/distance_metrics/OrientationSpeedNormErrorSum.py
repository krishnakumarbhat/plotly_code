import math

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature
from aspe.utilities.MathFunctions import normalize_angle_vector as nav


class OrientationSpeedNormErrorSum(IPairsFeature):

    def __init__(self, events_data, orientation='bounding_box_orientation', speed='speed', *args, **kwargs):
        """
        Init function realise class configuration: all metric parameters should be set here

        :param args: any parameters needed for metric calculation
        :param kwargs: same as above
        """
        super().__init__(*args, **kwargs)
        self.events_data = events_data
        self.orientation = orientation
        self.speed = speed

    def calculate(self, data_a: pd.DataFrame, data_b: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        """
        Function which calculates cartesian distance between two data frames using specified columns.

        :param data_a: DataFrame for which metric should be calculated, (should be same shape as data_b)
            for this function it should be referenced data
        :type data_a: pandas.DataFrame
        :param data_b: DataFrame for which metric should be calculated (should be same shape as data_a)
            for this function it should be estimated data
        :type data_b: pandas.DataFrame
        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as input data frame with distance values
        """
        # orientation and speed handle
        orientation_a = data_a.loc[:, self.orientation].values.astype('float32')
        orientation_b = data_b.loc[:, self.orientation].values.astype('float32')
        speed_a = data_a.loc[:, self.speed].values.astype('float32')
        speed_b = data_b.loc[:, self.speed].values.astype('float32')

        speed_error = abs(speed_a - speed_b)
        norm_speed_error = speed_error / self.events_data['Max Speed Error [m/s]']

        orientation_error = nav(abs(orientation_a - orientation_b))
        norm_orientation_error = orientation_error / math.radians(self.events_data['Max Heading Error [m/s]'])

        distance = norm_speed_error + norm_orientation_error
        return pd.DataFrame({
            'orientation_speed_norm_error_sum': distance,
        })
