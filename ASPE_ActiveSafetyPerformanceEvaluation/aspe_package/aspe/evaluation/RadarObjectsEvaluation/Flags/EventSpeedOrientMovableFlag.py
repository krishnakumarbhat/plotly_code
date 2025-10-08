import math

import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag
from aspe.evaluation.RadarObjectsEvaluation.Flags.IsMovableFlag import IsMovableFlag
from aspe.evaluation.RadarObjectsEvaluation.Flags.SignalEqualityFlag import SignalEqualityFlag
from aspe.extractors.Interfaces.Enums.Object import MovementStatus
from aspe.utilities.MathFunctions import normalize_angle_vector as nav


class EventSpeedOrientMovableFlag(IFlag):
    """
    Class for configurable flag calculation: check if speed and orientation are both below certain threshold
    """

    def __init__(self, events_data, max_speed_error=20, max_orientation_error=math.radians(90),
                 flag_signature='EventSpeedOrientMovableFlag', *args, **kwargs):
        """
        Setting initial parameters
        :param args:
        :param kwargs:
        """
        super().__init__(flag_signature, *args, **kwargs)
        self.max_orientation_error = max_orientation_error  # [rad]
        self.max_speed_error = max_speed_error  # [m/s]
        self.events_data = events_data

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        output = IsMovableFlag().calc_flag(data_frame)

        orientation_a = math.radians(self.events_data['Heading [deg]'])
        orientation_b = data_frame.loc[:, 'bounding_box_orientation'].values.astype('float32')
        speed_a = self.events_data['Speed [m/s]']
        speed_b = data_frame.loc[:, 'speed'].values.astype('float32')

        distance_speed_mask = abs(speed_a - speed_b) <= self.max_speed_error
        distance_orientation_mask = nav(abs(orientation_a - orientation_b)) <= self.max_orientation_error
        distance_mask = distance_speed_mask & distance_orientation_mask

        output_si = output.index
        output = output.reset_index(drop=True)
        output = output & pd.Series(distance_mask)
        output.index = output_si
        return output
