import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag


class SignalEqualityFlag(IFlag):
    """ Class for configurable flag calculation: check if signal is equal to given value """
    def __init__(self, signal_name, signal_expected_value, flag_signature='signal_equality', *args, **kwargs):
        """
        Setting initial parameters
        :param signal_name: Name of signal (column) in data frame
        :param signal_expected_value: expected value of signals
        :param args:
        :param kwargs:
        """
        super().__init__(flag_signature, *args, **kwargs)
        self.signal_name = signal_name
        self.signal_expected_value = signal_expected_value

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        f_valid = data_frame[self.signal_name] == self.signal_expected_value
        output = pd.Series(f_valid, index=data_frame.index)
        return output
