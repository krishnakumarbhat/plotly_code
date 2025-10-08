from abc import ABC, abstractmethod

import pandas as pd


class IFlag(ABC):

    def __init__(self, flag_signature=None, *args, **kwargs):
        """
        Init function realise class configuration: all flag parameters should be set here

        :param args: any parameters needed for flag calculation
        :param kwargs: same as above
        """
        self.flag_signature = flag_signature

    @abstractmethod
    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        raise NotImplementedError(
            'This method is an abstract -> please overwrite it or use or use one of the existing classes')

    def __and__(self, other):
        """ Overload operator with LogicalAndFlag """
        from aspe.evaluation.RadarObjectsEvaluation.Flags.LogicalAndFlag import LogicalAndFlag
        return LogicalAndFlag([self, other])
