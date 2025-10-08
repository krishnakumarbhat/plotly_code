from abc import ABC, abstractmethod

import pandas as pd


class IDetEvalPairsFeatures(ABC):

    @abstractmethod
    def calc_features(self, valid_pairs_synch_dets: pd.DataFrame,
                      valid_pairs_synch_objects: pd.DataFrame,
                      valid_pairs_synch_sensor: pd.DataFrame) \
            -> pd.DataFrame:
        """
        Calculating Features for each pair (detection, object)
        :param valid_pairs_synch_dets: detection signals synchronized in size with other inputs
        :type valid_pairs_synch_dets: pd.DataFrame
        :param valid_pairs_synch_objects: objects signals synchronized in size with other inputs
        :type valid_pairs_synch_objects: pd.DataFrame
        :param valid_pairs_synch_sensor: sensor signals synchronized in size with other inputs
        :type valid_pairs_synch_sensor: pd.DataFrame
        :return: pd.DataFrame DataFrame with features
        """

        raise NotImplementedError('This method is an abstract ->'
                                  ' please overwrite it or use one of the existing classes')
