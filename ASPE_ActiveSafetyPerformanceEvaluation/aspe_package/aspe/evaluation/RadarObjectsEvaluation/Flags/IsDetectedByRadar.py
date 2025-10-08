from itertools import chain, groupby

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag


class IsDetectedByRadar(IFlag):
    """
    Class for checking if objects were seen by radar. It requires precomputed column which contains number of associated
    radar detections.
    """

    def __init__(self,
                 flag_signature='is_detected_by_radar',
                 dets_count_column_name='n_dets',
                 max_coasting_num: int = 5,
                 visibility_rate_column_name='visibility_rate',
                 visibility_rate_low_thr=0.05,
                 visibility_rate_hi_thr=0.9,
                 *args, **kwargs):
        """
        Setting initial parameters
        :param args:
        :param kwargs:
        """
        super().__init__(flag_signature, *args, **kwargs)
        self.dets_count_column_name = dets_count_column_name
        self.max_coasting_num = max_coasting_num

        self.visibility_rate_column_name = visibility_rate_column_name
        self.visibility_rate_low_thr = visibility_rate_low_thr
        self.visibility_rate_hi_thr = visibility_rate_hi_thr

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        """
        :param data_frame: DataFrame for which flag should be calculated
        :type data_frame: pandas.DataFrame

        :param args:
        :param kwargs:
        :return: pandas.Series: - series with the same length as data frame with flag (boolean)
        """
        if self.dets_count_column_name in data_frame and self.visibility_rate_column_name in data_frame:
            output = pd.Series(data=np.full(len(data_frame), False), index=data_frame.index)
            for obj_unique_id, object in data_frame.groupby(by='unique_id'):

                dets_count = object.loc[:, self.dets_count_column_name]
                vis_rate = object.loc[:, self.visibility_rate_column_name]

                if np.any(dets_count):
                    """
                    Next line creates list of ints which represents how many occurances of same number in a row was 
                    found in dets_count. For example:
                    dets_count = [0, 0, 0, 1, 2, 2, 2, 0, 1, 1]
                    groupby operation groups it into: 000 1 222 0 11, and then len of each group is saved:
                    output = [3, 1, 3, 1, 2]
                    """
                    reapeted_occurs_in_row = [len(list(g)) for k, g in groupby(dets_count)]

                    """
                    Next line creates list which represents how many occurrences of given number in a row was 
                    already detected.:
                    dets_count          = [0, 0, 0, 1, 2, 2, 2, 0, 1, 1]
                    occurencies_counter = [1, 2, 3, 1, 1, 2, 3, 1, 1, 2]
                    """
                    occurencies_counter = np.array(
                        list(chain.from_iterable([range(1, c + 1) for c in reapeted_occurs_in_row])))
                    detected_first_time = np.argmax(dets_count > 0)

                    obj_was_detected = dets_count > 0
                    obj_was_detected |= ~obj_was_detected & (occurencies_counter < self.max_coasting_num)
                    obj_was_detected[:detected_first_time] = False

                    obj_was_detected[vis_rate > self.visibility_rate_hi_thr] = True
                    obj_was_detected[vis_rate < self.visibility_rate_low_thr] = False

                    output[dets_count.index] = obj_was_detected


        else:
            output = pd.Series(data=np.full(len(data_frame), True), index=data_frame.index)
        return output
