import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags import IFlag


class IsVisibleForRadarFlag(IFlag):
    class radar_names:
        RR = 'RR'
        RL = 'RL'
        FR = 'FR'
        FL = 'FL'
        CL = 'CL'
        CR = 'CR'

    def __init__(self, radar_name, threshold, flag_signature='is_visible_for_radar', *args, **kwargs):
        """
        Setting initial parameters
        :param args:
        :param kwargs:
        """
        super().__init__(flag_signature, *args, **kwargs)
        self.radar_name = radar_name
        self.threshold = threshold

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        occlusion_values = data_frame[f'SRR5-{self.radar_name}_visible_area'] / data_frame[
            f'SRR5-{self.radar_name}_total_area']
        output = occlusion_values.replace(np.nan, 0) >= self.threshold
        return pd.Series(output)
