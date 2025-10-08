import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.supporting_functions import (
    adjust_pos_by_reference_point_2_reference_point,
    calculate_nees_values_2d,
)
from aspe.utilities.MathFunctions import calc_velocity_in_position


class PositionNeesValuesR2R(IPairsFeature):
    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        """
        Calculates NEES values for position reference-to-reference (R2R).
        """
        adjust_pos_by_reference_point_2_reference_point(estimated_data, reference_data)
        dev_position_x = estimated_data['position_x'] - reference_data['position_x']
        dev_position_y = estimated_data['position_y'] - reference_data['position_y']

        position_variance_x = estimated_data['position_variance_x']
        position_variance_y = estimated_data['position_variance_y']
        position_covariance = estimated_data['position_covariance']

        nees_value_position_x, nees_value_position_y, nees_value_position_xy = \
            calculate_nees_values_2d(dev_position_x, dev_position_y, position_variance_x, position_variance_y,
                                     position_covariance)

        return pd.DataFrame({
            'nees_value_position_x': nees_value_position_x,
            'nees_value_position_y': nees_value_position_y,
            'nees_value_position_xy': nees_value_position_xy,
        })


class VelocityNeesValuesR2R(IPairsFeature):
    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        """
        Calculates NEES values for velocity reference-to-reference (R2R).
        """
        origin_ref_position = reference_data[{'position_x', 'position_y'}].copy()
        adjust_pos_by_reference_point_2_reference_point(estimated_data, reference_data)
        ref_vel_x, ref_vel_y = calc_velocity_in_position(origin_ref_position['position_x'],
                                                         origin_ref_position['position_y'],
                                                         reference_data['velocity_otg_x'],
                                                         reference_data['velocity_otg_y'],
                                                         reference_data['yaw_rate'],
                                                         reference_data['position_x'], reference_data['position_y'])
        dev_vel_x = ref_vel_x - estimated_data['velocity_otg_x']
        dev_vel_y = ref_vel_y - estimated_data['velocity_otg_y']

        velocity_otg_variance_x = estimated_data['velocity_otg_variance_x']
        velocity_otg_variance_y = estimated_data['velocity_otg_variance_y']
        velocity_otg_covariance = estimated_data['velocity_otg_covariance']

        nees_value_velocity_x, nees_value_velocity_y, nees_value_velocity_xy = \
            calculate_nees_values_2d(dev_vel_x, dev_vel_y, velocity_otg_variance_x, velocity_otg_variance_y,
                                     velocity_otg_covariance)

        return pd.DataFrame({
            'nees_value_velocity_x': nees_value_velocity_x,
            'nees_value_velocity_y': nees_value_velocity_y,
            'nees_value_velocity_xy': nees_value_velocity_xy,
        })
