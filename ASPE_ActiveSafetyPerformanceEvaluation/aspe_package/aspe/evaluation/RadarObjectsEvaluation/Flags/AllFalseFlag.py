import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag


class AllFalseFlag(IFlag):
    """ Dummy class to return always false """
    def __init__(self, flag_signature='all false'):
        super().__init__(flag_signature)

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        indexes = data_frame.index
        return pd.Series(False, index=indexes)
