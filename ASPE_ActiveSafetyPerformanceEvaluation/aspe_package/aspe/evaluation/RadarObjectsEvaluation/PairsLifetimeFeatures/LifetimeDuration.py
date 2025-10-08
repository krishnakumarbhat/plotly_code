import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures.IPairsLifetimeFeature import IPairsLifetimeFeature


class LifetimeDuration(IPairsLifetimeFeature):
    def calculate(self, estimated_object: pd.DataFrame, reference_object: pd.DataFrame, pair_data: pd.DataFrame,
                  *args, **kwargs) -> pd.DataFrame:
        lifetime_seconds = estimated_object['timestamp'].iloc[-1] - estimated_object['timestamp'].iloc[0]
        lifetime_samples = len(pair_data)
        results = {'lifetime_seconds': lifetime_seconds, 'lifetime_samples': lifetime_samples}
        return pd.DataFrame(results, index=[0])
