import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature
from aspe.utilities.MathFunctions import normalize_angle_vector

""" 
Used Acronyms: world will hate me for those =D
    R2R - Reference 2 reference point
    C2C - Center 2 center point 
"""


class SpeedDeviation(IPairsFeature):
    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        dev_speed = estimated_data['speed'] - reference_data['speed']
        return pd.DataFrame({
            'dev_speed': dev_speed,
        })


class OrientationDeviation(IPairsFeature):
    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        bb_orientation_est = estimated_data['bounding_box_orientation']
        bb_orientation_ref = reference_data['bounding_box_orientation']
        dev_orientation = normalize_angle_vector(bb_orientation_est - bb_orientation_ref)
        return pd.DataFrame({
            'dev_orientation': dev_orientation,
        })


class DimensionsDeviation(IPairsFeature):
    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        dev_len = estimated_data['bounding_box_dimensions_x'] - reference_data['bounding_box_dimensions_x']
        dev_wid = estimated_data['bounding_box_dimensions_y'] - reference_data['bounding_box_dimensions_y']
        return pd.DataFrame({
            'dev_bounding_box_dimensions_x': dev_len,
            'dev_bounding_box_dimensions_y': dev_wid,
        })


class YawRateDeviation(IPairsFeature):
    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        dev_yaw_rate = estimated_data['yaw_rate'] - reference_data['yaw_rate']
        return pd.DataFrame({
            'dev_yaw_rate': dev_yaw_rate,
        })