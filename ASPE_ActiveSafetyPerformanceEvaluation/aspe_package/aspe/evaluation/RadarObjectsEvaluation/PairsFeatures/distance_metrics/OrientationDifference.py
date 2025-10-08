import math

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature
from aspe.utilities.MathFunctions import normalize_angle_vector as nav


class OrientationDifference(IPairsFeature):

    def __init__(self, orientation='bounding_box_orientation', *args, **kwargs):
        """
        Init function realise class configuration: all metric parameters should be set here

        :param args: any parameters needed for metric calculation
        :param kwargs: same as above
        """
        super().__init__(*args, **kwargs)
        self.orientation = orientation

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
        orientation_error = nav(abs(orientation_a - orientation_b))
        orientation_error_deg = np.rad2deg(orientation_error)
        return pd.DataFrame({
            'orientation_error_deg': orientation_error_deg,
        })
