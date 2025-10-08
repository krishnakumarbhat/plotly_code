import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.IPairsFeature import IPairsFeature
from aspe.utilities.MathFunctions import (
    calc_bbox_closest_refernce_point,
    calc_position_in_bounding_box,
    calc_velocity_in_position,
)

""" 
Used Acronyms: world will hate me for those =D
    R2R - Reference 2 reference point
    C2C - Center 2 center point 
    CR2CR - closest reference 2 closes reference
"""


class PositionDeviationCR2CR(IPairsFeature):
    """ Closest reference 2  closest reference (CR2CR) deviation """

    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        closest_ref_long_point_of_ref, closest_ref_lat_point_of_ref = calc_bbox_closest_refernce_point(
            reference_data['center_x'],
            reference_data['center_y'],
            reference_data['bounding_box_dimensions_x'],
            reference_data['bounding_box_dimensions_y'],
            reference_data['bounding_box_orientation'])

        estimated_closest_ref_long_posn, estimated_closest_ref_lat_posn = calc_position_in_bounding_box(
            estimated_data['position_x'].to_numpy(),
            estimated_data['position_y'].to_numpy(),
            estimated_data['bounding_box_dimensions_x'].to_numpy(),
            estimated_data['bounding_box_dimensions_y'].to_numpy(),
            estimated_data['bounding_box_orientation'].to_numpy(),
            estimated_data['bounding_box_refpoint_long_offset_ratio'].to_numpy(),
            estimated_data['bounding_box_refpoint_lat_offset_ratio'].to_numpy(),
            closest_ref_long_point_of_ref,
            closest_ref_lat_point_of_ref)

        reference_closest_ref_long_posn, reference_closest_ref_lat_posn = calc_position_in_bounding_box(
            reference_data['position_x'].to_numpy(),
            reference_data['position_y'].to_numpy(),
            reference_data[
                'bounding_box_dimensions_x'].to_numpy(),
            reference_data[
                'bounding_box_dimensions_y'].to_numpy(),
            reference_data[
                'bounding_box_orientation'].to_numpy(),
            reference_data[
                'bounding_box_refpoint_long_offset_ratio'].to_numpy(),
            reference_data[
                'bounding_box_refpoint_lat_offset_ratio'].to_numpy(),
            closest_ref_long_point_of_ref,
            closest_ref_lat_point_of_ref)

        dev_position_x = estimated_closest_ref_long_posn - reference_closest_ref_long_posn
        dev_position_y = estimated_closest_ref_lat_posn - reference_closest_ref_lat_posn
        return pd.DataFrame({
            'dev_position_x': dev_position_x,
            'dev_position_y': dev_position_y,
        })


class VelocityDeviationCRC2R(IPairsFeature):
    """ Reference 2 reference (CR2CR) point deviation """

    def calculate(self, estimated_data: pd.DataFrame, reference_data: pd.DataFrame, *args, **kwargs) -> pd.DataFrame:
        closest_ref_long_point_of_ref, closest_ref_lat_point_of_ref = calc_bbox_closest_refernce_point(
            reference_data['center_x'],
            reference_data['center_y'],
            reference_data['bounding_box_dimensions_x'],
            reference_data['bounding_box_dimensions_y'],
            reference_data['bounding_box_orientation'])

        ref_vel_x, ref_vel_y = calc_velocity_in_position(reference_data['position_x'].to_numpy(),
                                                         reference_data['position_y'].to_numpy(),
                                                         reference_data['velocity_otg_x'].to_numpy(),
                                                         reference_data['velocity_otg_y'].to_numpy(),
                                                         reference_data['yaw_rate'].to_numpy(),
                                                         closest_ref_long_point_of_ref,
                                                         closest_ref_lat_point_of_ref)

        est_vel_x, est_vel_y = calc_velocity_in_position(estimated_data['position_x'].to_numpy(),
                                                         estimated_data['position_y'].to_numpy(),
                                                         estimated_data['velocity_otg_x'].to_numpy(),
                                                         estimated_data['velocity_otg_y'].to_numpy(),
                                                         estimated_data['yaw_rate'].to_numpy(),
                                                         closest_ref_long_point_of_ref,
                                                         closest_ref_lat_point_of_ref)
        dev_vel_x = ref_vel_x - est_vel_x
        dev_vel_y = ref_vel_y - est_vel_y
        return pd.DataFrame({
            'dev_velocity_x': dev_vel_x,
            'dev_velocity_y': dev_vel_y,
        })
