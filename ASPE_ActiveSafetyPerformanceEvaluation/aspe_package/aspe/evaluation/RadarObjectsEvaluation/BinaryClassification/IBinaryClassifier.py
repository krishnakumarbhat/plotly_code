from abc import ABC, abstractmethod
from enum import Enum

import pandas as pd


class BCType(Enum):
    NotAssigned = -1
    TruePositive = 0
    TrueNegative = 1
    FalsePositive = 2
    FalseNegative = 3
    OverSegmentation = 4
    UnderSegmentation = 5


class IBinaryClassifier(ABC):
    def __init__(self, *args, **kwargs):
        pass

    def initial_classification(self, data):
        data.signals.loc[:, 'binary_classification'] = BCType.NotAssigned

    @abstractmethod
    def classify_pairs(self, paired_data: pd.DataFrame, *args, **kwargs):
        pass

    @abstractmethod
    def classify_estimated_data(self, est_data: pd.DataFrame, *args, **kwargs):
        pass

    @abstractmethod
    def classify_reference_data(self, ref_data: pd.DataFrame, *args, **kwargs):
        pass
