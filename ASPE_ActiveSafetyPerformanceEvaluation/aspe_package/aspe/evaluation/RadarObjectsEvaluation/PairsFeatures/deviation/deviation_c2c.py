import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature

""" 
Used Acronyms: world will hate me for those =D
    R2R - Reference 2 reference point
    C2C - Center 2 center point 
"""


class PositionDeviationC2C(IPairsFeature):
    """ Center 2 center (C2C) deviation """
    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        dev_position_x = estimated_data['center_x'] - reference_data['center_x']
        dev_position_y = estimated_data['center_y'] - reference_data['center_y']
        return pd.DataFrame({
            'dev_position_x': dev_position_x,
            'dev_position_y': dev_position_y,
        })
