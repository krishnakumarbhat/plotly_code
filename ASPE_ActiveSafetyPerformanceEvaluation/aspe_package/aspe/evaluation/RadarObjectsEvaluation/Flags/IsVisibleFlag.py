import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags import IFlag, IsVisibleForRadarFlag


class IsVisibleFlag(IFlag):

    def __init__(self, threshold, variant='mid', flag_signature='is_visible', *args, **kwargs):
        """
        Setting initial parameters
        :param args:
        :param kwargs:
        """
        super().__init__(flag_signature, *args, **kwargs)
        variant = variant.lower()
        radars = IsVisibleForRadarFlag.radar_names
        self.radar_visibility_flags = []
        if variant in ('low', 'mid', 'high'):
            self.radar_visibility_flags.extend([
                IsVisibleForRadarFlag(radars.FL, threshold),
                IsVisibleForRadarFlag(radars.FR, threshold),
            ])
        if variant in ('mid', 'high'):
            self.radar_visibility_flags.extend([
                IsVisibleForRadarFlag(radars.RL, threshold),
                IsVisibleForRadarFlag(radars.RR, threshold),
            ])
        if variant in ('high',):
            self.radar_visibility_flags.extend([
                IsVisibleForRadarFlag(radars.CL, threshold),
                IsVisibleForRadarFlag(radars.CR, threshold),
            ])

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """

        output = pd.Series(np.full(len(data_frame), False))
        for single_radar_visible_flag in self.radar_visibility_flags:
            output = output | single_radar_visible_flag.calc_flag(data_frame)
        return output
