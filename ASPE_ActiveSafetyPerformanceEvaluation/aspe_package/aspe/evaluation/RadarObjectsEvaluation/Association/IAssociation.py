from abc import ABC, abstractmethod
from typing import Optional, Tuple

import pandas as pd


class IAssociation(ABC):
    """
    Class responsible for estimate to reference association. It operates on already pre-paired objects and decides,
    which pairs are can be treated as associated.
    """
    @abstractmethod
    def associate(self, pe_pairs: pd.DataFrame, pairs_est: pd.DataFrame, pairs_ref: pd.DataFrame) \
            -> Tuple[pd.Series, Optional[pd.DataFrame]]:
        """
        Main method for association.
        :param pe_pairs: DataFrame with information about pairs - each row of it represents estimate-reference pair.
        It is 'signal' attribute of PEPairedObjects class which is one of the output of
        IPEPairedObjectsPreBuilder.build() method
        :param pairs_est: contains all available signals of estimated objects which were paired. Same dimension as
        pe_pairs - 1st row of pairs_est contains information about estimated object's signals which is part of the pair
        in 1st row of pe_pairs
        :param pairs_ref: contains all available signals of reference objects which were paired. Same dimension as
        pe_pairs - 1st row of pairs_ref contains information about reference object's signals which is part of the pair
        in 1st row of pe_pairs
        :return: tuple(is_associated, additional_info):
        - is_associated: pd.Series of bools, same len as pe_pairs rows count. Represents which of pe_pairs can be
        treated as associated
        - additional_info: pd.DataFrame or None - if association calculated some additional information or features
        this should be placed here
        """
        pass
