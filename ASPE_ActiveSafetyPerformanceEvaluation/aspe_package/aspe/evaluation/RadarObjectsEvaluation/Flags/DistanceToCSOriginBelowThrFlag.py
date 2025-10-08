import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag


class DistanceToCSOriginBelowThrFlag(IFlag):
    """ Class for configurable flag calculation: check if something is closer or equal to coordinate system origin
        than given threshold """
    def __init__(self, pos_x_signal_name='center_x', pos_y_signal_name='center_y', thr=np.inf,
                 flag_signature='distance_to_cs_origin_below_thr', *args, **kwargs):
        """
        Setting initial parameters
        :param pos_x_signal_name: Name of signal (column) in data frame for x position
        :param pos_y_signal_name: Name of signal (column) in data frame for y position
        :param thr: threshold - maximum allowed value
        :param kwargs:
        """
        super().__init__(flag_signature, *args, **kwargs)
        self.pos_x_signal_name = pos_x_signal_name
        self.pos_y_signal_name = pos_y_signal_name
        self.thr = thr

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        pos_x = data_frame.loc[:, self.pos_x_signal_name].values.astype('float32')
        pos_y = data_frame.loc[:, self.pos_y_signal_name].values.astype('float32')
        distance = np.hypot(pos_x, pos_y)
        f_valid = distance <= self.thr
        output = pd.Series(f_valid, index=data_frame.index)
        return output
