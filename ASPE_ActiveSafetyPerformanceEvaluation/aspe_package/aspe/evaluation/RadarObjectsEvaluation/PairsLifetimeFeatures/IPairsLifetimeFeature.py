from abc import ABC, abstractmethod

import pandas as pd


class IPairsLifetimeFeature(ABC):
    """
    Interface for feature calculation for pair within its whole lifetime. Implement this for new feature development.
    """
    @abstractmethod
    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, pairs_est_ref: pd.DataFrame,
                  *args, **kwargs) -> pd.DataFrame:
        """
        Main method for feature calculation. Client object should prepare input DataFrames to be same size and paired
        row-by-row. Function should take only single unique pair as an input
        :param estimated_data: DataFrame which represents signals of estimated objects - single pair
        :param reference_data: DataFrame which represents signals of reference objects - single pair
        :param pairs_est_ref: DataFrame which represents signals of paired objects
        :param args: positional arguments
        :param kwargs: keyword arguments
        :return: pd.DataFrame with feature values, should be same length as input DataFrames
        """
        pass
