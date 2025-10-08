from copy import deepcopy
import pandas as pd
import numpy as np
import pytest
from aspe.extractors.F360.DataSets.F360Objects import F360Objects
from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtObjects import RtObjects
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtHost import RtHost
from aspe.utilities.MathFunctions import calc_position_in_bounding_box, cart2pol, calc_velocity_in_position

from aspe.extractors.F360.DataSets.F360RadarSensors import F360RadarSensors

from aspe.extractors.Interfaces.Enums.Object import MovementStatus


def create_ref(list_of_objects):

    ref_columns = ['scan_index', 'timestamp', 'unique_id',
                   'position_x', 'position_y', 'velocity_otg_x', 'velocity_otg_y',
                   'bounding_box_orientation', 'bounding_box_dimensions_x', 'bounding_box_dimensions_y',
                   'bounding_box_refpoint_lat_offset_ratio', 'bounding_box_refpoint_long_offset_ratio',
                   'yaw_rate', 'movement_status']

    input_df = pd.DataFrame(data=list_of_objects, columns=ref_columns)

    # Assign additional values
    add_additional_values_to_object_signals(input_df)

    # Prepare extracted data
    ref_data = ExtractedData()
    ref_data.objects = RtObjects()
    ref_data.host = RtHost()
    ref_data.objects.signals = pd.concat([input_df, ref_data.objects.signals],
                                                         axis=0, sort=False)

    return ref_data


def add_additional_values_to_object_signals(input_df):
    center_x, center_y = calc_position_in_bounding_box(
        input_df['position_x'].values, input_df['position_y'].values,
        input_df['bounding_box_dimensions_x'].values, input_df['bounding_box_dimensions_y'].values,
        input_df['bounding_box_orientation'].values,
        input_df['bounding_box_refpoint_long_offset_ratio'].values,
        input_df['bounding_box_refpoint_lat_offset_ratio'].values,
        np.array(0.5), np.array(0.5))
    input_df['center_x'] = center_x
    input_df['center_y'] = center_y
    speed, heading = cart2pol(input_df['velocity_otg_x'].values, input_df['velocity_otg_y'].values)
    input_df['speed'] = speed
    input_df['heading'] = heading


def change_ref_point(input_df,  out_ref_point_long, out_ref_point_lat):
    new_ref_point_x, new_ref_point_y = calc_position_in_bounding_box(
        input_df['position_x'].values, input_df['position_y'].values,
        input_df['bounding_box_dimensions_x'].values, input_df['bounding_box_dimensions_y'].values,
        input_df['bounding_box_orientation'].values,
        input_df['bounding_box_refpoint_long_offset_ratio'].values,
        input_df['bounding_box_refpoint_lat_offset_ratio'].values,
        out_ref_point_long, out_ref_point_lat)

    new_ref_point_vel_x, new_ref_point_vel_y = calc_velocity_in_position(
        input_df['position_x'].values, input_df['position_y'].values,
        input_df['velocity_otg_x'].values, input_df['velocity_otg_y'].values,
        input_df['yaw_rate'].values,
        new_ref_point_x, new_ref_point_y)

    input_df['position_x'] = new_ref_point_x
    input_df['position_y'] = new_ref_point_y
    input_df['velocity_otg_x'] = new_ref_point_vel_x
    input_df['velocity_otg_y'] = new_ref_point_vel_y
    input_df['bounding_box_refpoint_long_offset_ratio'] = out_ref_point_long
    input_df['bounding_box_refpoint_lat_offset_ratio'] = out_ref_point_lat
    speed, heading = cart2pol(new_ref_point_vel_x, new_ref_point_vel_y)
    input_df['speed'] = speed
    input_df['heading'] = heading


def join_two_ext_data(ref1, ref2):
    ref_out = deepcopy(ref1)
    ref_out.objects.signals = pd.concat([ref1.objects.signals, ref2.objects.signals],
                                                         axis=0, sort=False)
    ref_out.objects.signals.sort_values(by='scan_index', inplace=True)
    ref_out.objects.signals.reset_index(drop=True, inplace=True)
    return ref_out


def rt_object2f360_object(rt_object_signals):

    f360_objects_data_set = F360Objects()
    f360_objects_signals = f360_objects_data_set.signals
    rt_columns = rt_object_signals.columns
    rt_additional_columns = rt_columns.difference(f360_objects_signals.columns)
    f360_objects_signals = pd.concat([f360_objects_signals, rt_object_signals],
                                              axis=0, sort=False)
    f360_objects_signals.drop(columns=rt_additional_columns)
    return f360_objects_signals


def create_ideal_est_from_ref(ref_data):

    # Prepare extracted data
    est_data = ExtractedData()
    est_data.objects = F360Objects()
    est_data.host = F360RadarSensors()
    est_data.objects.signals = rt_object2f360_object(ref_data.objects.signals)
    return est_data


def shift_objects(object_signals, **kwargs):
    for signal_name, shift_value in kwargs.items():
        object_signals.loc[:, signal_name] += shift_value
    # recalculate dependent signals
    add_additional_values_to_object_signals(object_signals)
    return object_signals


def shift_objects_by_indexes(object_signals, indexes, **kwargs):
    for signal_name, shift_value in kwargs.items():
        object_signals.loc[indexes, signal_name] += shift_value
    # recalculate dependent signals
    add_additional_values_to_object_signals(object_signals)
    return object_signals


def convert_test_vector_to_pytest_param(test_vectors):
    pytest_param_list = list()
    for single_tv_name, single_tv in test_vectors.items():
        pytest_param_list.append(pytest.param(single_tv['est_data'], single_tv['ref_data'], id=single_tv_name))
    return pytest_param_list


def convert_test_vector_to_pytest_param_with_kpis(test_vectors):
    pytest_param_list = list()
    for single_tv_name, single_tv in test_vectors.items():
        pytest_param_list.append(pytest.param(single_tv['est_data'],
                                              single_tv['ref_data'],
                                              single_tv['expected_kpis'],
                                              id=single_tv_name))
    return pytest_param_list

