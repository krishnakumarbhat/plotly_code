import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import IPairsFeature


class GroundTruthObjectType(IPairsFeature):
    """
    Simple class which add column about ground truth object type (which should be stored in 'object_class' signals
    dataframe's column).
    """
    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:

        if 'object_class' in reference_data:
            return reference_data.loc[:, 'object_class']
        else:
            return pd.DataFrame(data={'object_class': np.full(len(estimated_data), fill_value=np.nan)},
                                index=reference_data.index)
