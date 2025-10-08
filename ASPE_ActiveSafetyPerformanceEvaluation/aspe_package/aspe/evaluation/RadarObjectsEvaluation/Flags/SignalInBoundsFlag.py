import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag


class SignalInBoundsFlag(IFlag):
    """ Class for configurable flag calculation: check if signal is within given bounds - bounds are included """

    def __init__(self, signal_name, min_value=-np.inf, max_value=np.inf, flag_signature='signal_in_bounds', *args,
                 **kwargs):
        """
        Setting initial parameters
        :param signal_name: Name of signal (column) in data frame
        :param min_value: minimum allowed value
        :param max_value: maximum allowed value
        :param kwargs:
        """
        super().__init__(flag_signature, *args, **kwargs)
        self.signal_name = signal_name
        self.min_value = min_value
        self.max_value = max_value

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """

        f_valid = np.logical_and(data_frame[self.signal_name].values >= self.min_value,
                                 data_frame[self.signal_name].values <= self.max_value)
        output = pd.Series(f_valid, index=data_frame.index)
        return output
