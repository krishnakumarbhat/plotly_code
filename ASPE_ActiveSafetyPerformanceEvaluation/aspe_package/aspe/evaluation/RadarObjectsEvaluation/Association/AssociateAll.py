from typing import Optional, Tuple

import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Association import IAssociation


class AssociateAll(IAssociation):
    def associate(self, pe_pairs: pd.DataFrame, pairs_est: pd.DataFrame, pairs_ref: pd.DataFrame) \
            -> Tuple[pd.Series, Optional[pd.DataFrame]]:
        return pd.Series(True, index=pe_pairs.index), None
