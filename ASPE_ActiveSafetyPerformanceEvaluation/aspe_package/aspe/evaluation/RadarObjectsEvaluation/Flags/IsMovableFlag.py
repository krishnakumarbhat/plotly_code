import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag
from aspe.evaluation.RadarObjectsEvaluation.Flags.SignalEqualityFlag import SignalEqualityFlag
from aspe.extractors.Interfaces.Enums.Object import MovementStatus


class IsMovableFlag(IFlag):
    """ Class for configurable flag calculation: check if object is movable:
        movement_status equal to MOVING or STOPPED """
    def __init__(self, flag_signature='is_movable', *args, **kwargs):
        """
        Setting initial parameters
        :param args:
        :param kwargs:
        """
        super().__init__(flag_signature, *args, **kwargs)
        self.is_moving_flag = SignalEqualityFlag(signal_name='movement_status',
                                                 signal_expected_value=MovementStatus.MOVING)
        self.is_stopped_flag = SignalEqualityFlag(signal_name='movement_status',
                                                  signal_expected_value=MovementStatus.STOPPED)

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        output = self.is_moving_flag.calc_flag(data_frame) | self.is_stopped_flag.calc_flag(data_frame)
        return output
