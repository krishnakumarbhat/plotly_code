import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.supporting_functions import (
    adjust_pos_by_reference_point_2_reference_point,
)
from aspe.utilities.MathFunctions import calc_acceleration_in_position, calc_velocity_in_position

""" 
Used Acronyms: world will hate me for those =D
    R2R - Reference 2 reference point
    C2C - Center 2 center point 
    CR2CR - Closest reference 2 closest reference point
"""


class PositionDeviationR2R(IPairsFeature):
    """ Reference 2 reference (R2R) point deviation """

    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        adjust_pos_by_reference_point_2_reference_point(estimated_data, reference_data)
        dev_position_x = estimated_data['position_x'] - reference_data['position_x']
        dev_position_y = estimated_data['position_y'] - reference_data['position_y']
        return pd.DataFrame({
            'dev_position_x': dev_position_x,
            'dev_position_y': dev_position_y,
        })


class VelocityDeviationR2R(IPairsFeature):
    """ Reference 2 reference (R2R) point deviation """

    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
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
        return pd.DataFrame({
            'dev_velocity_x': dev_vel_x,
            'dev_velocity_y': dev_vel_y,
        })


class VelocityRelativeDeviationR2R(IPairsFeature):
    """ Reference 2 reference (R2R) point deviation """

    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        origin_ref_position = reference_data[{'position_x', 'position_y'}].copy()
        adjust_pos_by_reference_point_2_reference_point(estimated_data, reference_data)
        ref_vel_x, ref_vel_y = calc_velocity_in_position(origin_ref_position['position_x'],
                                                         origin_ref_position['position_y'],
                                                         reference_data['velocity_rel_x'],
                                                         reference_data['velocity_rel_y'],
                                                         reference_data['yaw_rate'],
                                                         reference_data['position_x'], reference_data['position_y'])
        dev_vel_x = ref_vel_x - estimated_data['velocity_rel_x']
        dev_vel_y = ref_vel_y - estimated_data['velocity_rel_y']

        return pd.DataFrame({
            'dev_velocity_rel_x': dev_vel_x,
            'dev_velocity_rel_y': dev_vel_y,
        })


class AccelerationDeviationR2R(IPairsFeature):
    """ Reference 2 reference (R2R) point deviation """

    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        origin_ref_position = reference_data[{'position_x', 'position_y'}].copy()
        adjust_pos_by_reference_point_2_reference_point(estimated_data, reference_data)
        try:
            ref_acc_x, ref_acc_y = calc_acceleration_in_position(origin_ref_position['position_x'],
                                                                 origin_ref_position['position_y'],
                                                                 reference_data['acceleration_otg_x'],
                                                                 reference_data['acceleration_otg_y'],
                                                                 reference_data['yaw_rate'],
                                                                 reference_data['yaw_acceleration'],
                                                                 reference_data['position_x'],
                                                                 reference_data['position_y'])
            dev_acc_x = ref_acc_x - estimated_data['acceleration_otg_x']
            dev_acc_y = ref_acc_y - estimated_data['acceleration_otg_y']
        except KeyError:
            dev_acc_x = np.full(len(estimated_data), np.nan)
            dev_acc_y = np.full(len(estimated_data), np.nan)
        return pd.DataFrame({
            'dev_acceleration_otg_x': dev_acc_x,
            'dev_acceleration_otg_y': dev_acc_y,
        })


class AccelerationRelativeDeviationR2R(IPairsFeature):
    """ Reference 2 reference (R2R) point deviation """

    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        origin_ref_position = reference_data[{'position_x', 'position_y'}].copy()
        adjust_pos_by_reference_point_2_reference_point(estimated_data, reference_data)
        try:
            ref_acc_x, ref_acc_y = calc_acceleration_in_position(origin_ref_position['position_x'],
                                                                 origin_ref_position['position_y'],
                                                                 reference_data['acceleration_rel_x'],
                                                                 reference_data['acceleration_rel_y'],
                                                                 reference_data['yaw_rate'],
                                                                 reference_data['yaw_acceleration'],
                                                                 reference_data['position_x'],
                                                                 reference_data['position_y'])
            dev_acc_x = ref_acc_x - estimated_data['acceleration_rel_x']
            dev_acc_y = ref_acc_y - estimated_data['acceleration_rel_y']
        except KeyError:
            dev_acc_x = np.full(len(estimated_data), np.nan)
            dev_acc_y = np.full(len(estimated_data), np.nan)

        return pd.DataFrame({
            'dev_acceleration_rel_x': dev_acc_x,
            'dev_acceleration_rel_y': dev_acc_y,
        })
