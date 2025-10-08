import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature


class CartesianDistance2D(IPairsFeature):

    def __init__(self, pos_x_signal_name='center_x', pos_y_signal_name='center_y', *args, **kwargs):
        """
        Init function realise class configuration: all metric parameters should be set here

        :param args: any parameters needed for metric calculation
        :param kwargs: same as above
        """
        super().__init__(*args, **kwargs)
        self.pos_x_signal_name = pos_x_signal_name
        self.pos_y_signal_name = pos_y_signal_name

    def calculate(self, data_a: pd.DataFrame, data_b: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        """
        Function which calculates cartesian distance between two data frames using specified columns.

        :param data_a: DataFrame for which metric should be calculated, (should be same shape as data_b)
        :type data_a: pandas.DataFrame
        :param data_b: DataFrame for which metric should be calculated (should be same shape as data_a)
        :type data_b: pandas.DataFrame
        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as input data frame with distance values
        """
        pos_x_a = data_a.loc[:, self.pos_x_signal_name].values.astype('float32')
        pos_x_b = data_b.loc[:, self.pos_x_signal_name].values.astype('float32')
        pos_y_a = data_a.loc[:, self.pos_y_signal_name].values.astype('float32')
        pos_y_b = data_b.loc[:, self.pos_y_signal_name].values.astype('float32')
        distance = pd.Series(np.hypot(pos_x_a - pos_x_b, pos_y_a - pos_y_b), index=data_a.index)
        return pd.DataFrame({
            'cartesian_distance_2d': distance,
        })
