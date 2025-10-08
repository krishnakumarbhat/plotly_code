from abc import ABC, abstractmethod

import pandas as pd


class IPairsFeature(ABC):
    """
    Interface for feature calculation between paired objects. Implement this for new feature development.
    """
    @abstractmethod
    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        """
        Main method for feature calculation. Client object should prepare input DataFrames to be same size and paired
        row-by-row.
        :param estimated_data: DataFrame which represents signals of estimated objects
        :param reference_data: DataFrame which represents signals of reference objects
        :param args: positional arguments
        :param kwargs: keyword arguments
        :return: pd.DataFrame with feature values, should be same length as input DataFrames
        """
        pass
