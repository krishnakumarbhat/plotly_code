from copy import deepcopy
from typing import List

import numpy as np
import pandas as pd
import pytest

from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors
from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Interfaces.IRadarDetections import IRadarDetections
from aspe.extractors.Interfaces.IObjects import IObjects

from aspe.extractors.Transform.objects import calc_object_rel_state_at_ref_point

from aspe.extractors.Transform.CrossDataSet.object2det import cart_rel_state_to_polar_state

from aspe.utilities.MathFunctions import pol2cart, vcs2scs, rot_2d_sae_cs
from aspe.utilities.data_filters import slice_df_based_on_other
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors import default_data


def get_default_sensors():
    sensors = IRadarSensors()
    sensors.signals = pd.DataFrame([default_data.sensor_series_zero_motion])
    sensors.per_sensor = default_data.default_sensor_per_sensor.copy()
    sensors.per_look = default_data.default_sensor_per_look.copy()
    sensors.coordinate_system = 'VCS'
    return sensors


def get_multi_bounce_detection(object_series, sensors, ref_point_x, ref_point_y, sensor_id, bounce_factor):
    detection = get_detection_on_state(object_series, sensors, ref_point_x, ref_point_y, sensor_id)
    detection['range'] *= bounce_factor
    detection['range_rate'] *= bounce_factor
    return detection


def get_wheel_spin_detection(object_series, sensors, ref_point_x, ref_point_y, sensor_id, velocity_multiplication):
    object_series_wheel_spin = deepcopy(object_series)
    object_series_wheel_spin['velocity_otg_x'] *= velocity_multiplication
    object_series_wheel_spin['velocity_otg_y'] *= velocity_multiplication

    detection = get_detection_on_state(object_series_wheel_spin, sensors, ref_point_x, ref_point_y, sensor_id)
    return detection


def get_detection_on_state(object_series_in, sensors, ref_point_x, ref_point_y, sensor_id):
    object_series = deepcopy(object_series_in)
    sensors_single_radar = deepcopy(sensors)
    sensors_single_radar.filter_single_radar(sensor_id)
    scs_location = sensors_single_radar.per_sensor.loc[0, :]
    sensor_motion = sensors.signals.loc[0, :].copy()

    # move to SCS
    object_series['position_x'], object_series['position_y'] = vcs2scs(object_series['position_x'],
                                                                       object_series['position_y'],
                                                                       scs_location.position_x,
                                                                       scs_location.position_y,
                                                                       scs_location.boresight_az_angle)

    object_series['velocity_otg_x'], object_series['velocity_otg_y'] = rot_2d_sae_cs(object_series['velocity_otg_x'],
                                                                                     object_series['velocity_otg_y'],
                                                                                     scs_location.boresight_az_angle)

    object_series['bounding_box_orientation'] -= scs_location.boresight_az_angle

    sensor_motion['velocity_otg_x'], sensor_motion['velocity_otg_y'] = rot_2d_sae_cs(sensor_motion['velocity_otg_x'],
                                                                                     sensor_motion['velocity_otg_y'],
                                                                                     scs_location.boresight_az_angle)

    rel_state = calc_object_rel_state_at_ref_point(object_series, sensor_motion, ref_point_x, ref_point_y)

    detection = default_data.default_detection.copy()
    detection.update(cart_rel_state_to_polar_state(rel_state))
    detection['sensor_id'] = sensor_id
    return detection


def build_detections(detections_signals, scs_location):
    detections = IRadarDetections()
    detections.signals = pd.DataFrame(detections_signals)
    scs_location_synch = slice_df_based_on_other(scs_location, detections.signals, ['sensor_id'], ['sensor_id'])
    detections.signals['unique_id'] = np.arange(len(detections_signals))
    detections.signals['position_x'], detections.signals['position_y'] = pol2cart(detections.signals['range'],
                                                                                  detections.signals['azimuth'])

    detections.coordinate_system = 'SCS'
    detections.scs2vcs(scs_location_synch)
    detections.signals['azimuth_vcs'] = detections.signals['azimuth'] + scs_location_synch.boresight_az_angle
    return detections


def build_gt_data(detections_signals, objects_signals, sensors):
    gt = ExtractedData()
    gt.detections = build_detections(detections_signals, sensors.per_sensor)

    gt.sensors = deepcopy(sensors)

    gt.objects = IObjects()
    gt.objects.signals = pd.DataFrame(objects_signals)
    gt.objects.signals['unique_id'] = np.arange(len(objects_signals))
    gt.objects.coordinate_system = 'VCS'

    return gt


def build_ref_data(objects_signals, sensors):
    ref = ExtractedData()

    ref.sensors = deepcopy(sensors)

    ref.objects = IObjects()
    ref.objects.signals = pd.DataFrame(objects_signals)
    ref.objects.signals['unique_id'] = np.arange(len(objects_signals))
    ref.objects.coordinate_system = 'VCS'
    return ref


def build_est_data(detections_signals, sensors):
    est = ExtractedData()
    est.detections = build_detections(detections_signals, sensors.per_sensor)
    return est


def build_test_vector(description_long, description_short, reference_data, estimated_data,
                      ground_truth_data=None,
                      expectation=None):
    data = {
        'description_long': description_long,
        'description_short': description_short,
        'ground_truth_data': ground_truth_data,
        'reference_data': reference_data,
        'estimated_data': estimated_data,
        'expectation': expectation,
    }
    return data


def convert_test_vector_to_pytest_param_est_ref(test_vectors: List):
    pytest_param_list = list()
    for single_tv in test_vectors:
        pytest_param_list.append(pytest.param(single_tv['estimated_data'],
                                              single_tv['reference_data'],
                                              id=single_tv['description_short']))
    return pytest_param_list


def convert_test_vector_to_pytest_param_est_ref_exp(test_vectors: List):
    pytest_param_list = list()
    for single_tv in test_vectors:
        pytest_param_list.append(pytest.param(single_tv['estimated_data'],
                                              single_tv['reference_data'],
                                              single_tv['expectation'],
                                              id=single_tv['description_short']))
    return pytest_param_list



