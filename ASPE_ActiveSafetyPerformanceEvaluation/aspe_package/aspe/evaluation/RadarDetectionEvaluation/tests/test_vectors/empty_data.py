import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Interfaces.IRadarDetections import IRadarDetections
from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors import default_data

from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.support_functions import build_gt_data, \
    build_ref_data, build_est_data, get_default_sensors, get_detection_on_state, build_test_vector


def get_empty_data():

    gt = ExtractedData()
    gt.detections = IRadarDetections()
    gt.objects = IObjects()
    gt.sensors = IRadarSensors()
    ref = ExtractedData()
    ref.objects = IObjects()
    ref.sensors = IRadarSensors()
    est = ExtractedData()
    est.detections = IRadarDetections()
    est.sensors = IRadarSensors()

    data = build_test_vector(description_long='Empty Data',
                             description_short='Empty Data',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt)

    return data


def get_2_radars_one_with_no_det_data():
    first_sensors = get_default_sensors()
    second_sensor = get_default_sensors()
    second_sensor.signals['sensor_id'] = default_data.front_right_sensor_id
    second_sensor.per_sensor['sensor_id'] = default_data.front_right_sensor_id
    second_sensor.per_look['sensor_id'] = default_data.front_right_sensor_id
    gt_sensors = IRadarSensors()
    gt_sensors.signals = pd.concat([first_sensors.signals, second_sensor.signals])
    gt_sensors.per_sensor = pd.concat([first_sensors.per_sensor, second_sensor.per_sensor])
    gt_sensors.per_look = pd.concat([first_sensors.per_look, second_sensor.per_look])
    gt_sensors.coordinate_system = 'VCS'

    gt_object = default_data.default_object.copy()
    gt_object.update(pd.Series({
        'position_x': 5.0,
        'position_y': 5.0,
        'velocity_otg_x': 5.0,
        'velocity_otg_y': 0.0,
        'bounding_box_dimensions_x': 4.0,
        'bounding_box_dimensions_y': 2.0,
        'center_x': 5.0,
        'center_y': 5.0,
    }))
    gt_detection = get_detection_on_state(gt_object, first_sensors, 0.0, 0.0, 1)

    gt = build_gt_data([gt_detection], [gt_object], gt_sensors)
    ref = build_ref_data([gt_object], gt_sensors)
    est = build_est_data([gt_detection], gt_sensors)

    data = build_test_vector(description_long='Two Sensors, only one with det',
                             description_short='Two valid sensors one det',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt)

    return data