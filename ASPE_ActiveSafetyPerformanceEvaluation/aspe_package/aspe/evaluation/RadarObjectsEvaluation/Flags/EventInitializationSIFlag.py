import math

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag
from aspe.evaluation.RadarObjectsEvaluation.Flags.SignalEqualityFlag import SignalEqualityFlag
from aspe.extractors.Interfaces.Enums.Object import MovementStatus


class EventInitializationSIFlag(IFlag):
    """
    Class for configurable flag calculation: setting relevant flag when object should be initialized
    """
    def __init__(self, events_data, flag_signature='EventInitializationSIFlag', *args, **kwargs):
        """
        Setting initial parameters
        :param args:
        :param kwargs:
        """
        self.events_data = events_data
        super().__init__(flag_signature, *args, **kwargs)

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        start_idx = self.events_data['Tracker Idx Start']
        init_idx = self.events_data['Initialization Scan Index']
        end_idx = self.events_data['Tracker Idx End']

        rel_mask = pd.Series(np.zeros(data_frame.shape[0]), index=np.arange(start_idx, end_idx + 1), dtype=bool)
        rel_mask[init_idx:end_idx] = True

        rel_mask.index = data_frame.index
        return rel_mask
