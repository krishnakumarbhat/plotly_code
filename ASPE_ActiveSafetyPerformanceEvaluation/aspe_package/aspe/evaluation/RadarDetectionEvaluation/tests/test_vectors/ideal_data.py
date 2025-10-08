import numpy as np
import pandas as pd

from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors import default_data
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.support_functions import \
    get_default_sensors, build_gt_data, build_ref_data, build_est_data, build_test_vector, get_detection_on_state


def get_single_object_with_single_ideal_detection():

    gt_sensors = get_default_sensors()
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
    gt_detection = get_detection_on_state(gt_object, gt_sensors, 0.0, 0.0, 1)

    gt = build_gt_data([gt_detection], [gt_object], gt_sensors)
    ref = build_ref_data([gt_object], gt_sensors)
    est = build_est_data([gt_detection], gt_sensors)

    data = build_test_vector(description_long='Ideal single object and ideal single detection',
                             description_short='Ideal 1 ref, 1 est',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt)

    return data


def get_single_object_with_three_ideal_detection():

    gt_sensors = get_default_sensors()
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
    gt_detections = list()
    gt_detections.append(get_detection_on_state(gt_object, gt_sensors, 0.0, 0.0, 1))
    gt_detections.append(get_detection_on_state(gt_object, gt_sensors, 1.0, 0.0, 1))
    gt_detections.append(get_detection_on_state(gt_object, gt_sensors, 0.0, 1.0, 1))

    gt = build_gt_data(gt_detections, [gt_object], gt_sensors)
    ref = build_ref_data([gt_object], gt_sensors)
    est = build_est_data(gt_detections, gt_sensors)

    data = build_test_vector(description_long='Ideal single object and ideal three detection at corner',
                             description_short='Ideal 1 ref, 3 est, corner',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt)

    return data


def get_single_object_with_three_ideal_detection_inside():

    gt_sensors = get_default_sensors()
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
    gt_detections = list()
    gt_detections.append(get_detection_on_state(gt_object, gt_sensors, 0.2, 0.2, 1))
    gt_detections.append(get_detection_on_state(gt_object, gt_sensors, 0.5, 0.3, 1))
    gt_detections.append(get_detection_on_state(gt_object, gt_sensors, 0.1, 0.9, 1))

    gt = build_gt_data(gt_detections, [gt_object], gt_sensors)
    ref = build_ref_data([gt_object], gt_sensors)
    est = build_est_data(gt_detections, gt_sensors)

    data = build_test_vector(description_long='Ideal single object and ideal three detection inside object',
                             description_short='Ideal 1 ref, 3 est, inside',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt)

    return data


def get_two_object_with_three_ideal_detection_inside_each():

    gt_sensors = get_default_sensors()
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
    gt_objects = [gt_object.copy()]
    gt_object.update(pd.Series({
        'position_x': 4.0,
        'position_y': -7.0,
        'velocity_otg_x': 5.0,
        'velocity_otg_y': -5.0,
        'bounding_box_orientation': np.deg2rad(-45.0),
        'bounding_box_dimensions_x': 4.0,
        'bounding_box_dimensions_y': 2.0,
        'center_x': 4.0,
        'center_y': -7.0,
    }))
    gt_objects.append(gt_object)

    gt_detections = list()
    for single_object in gt_objects:
        gt_detections.append(get_detection_on_state(single_object, gt_sensors, 0.2, 0.2, 1))
        gt_detections.append(get_detection_on_state(single_object, gt_sensors, 0.5, 0.3, 1))
        gt_detections.append(get_detection_on_state(single_object, gt_sensors, 0.1, 0.9, 1))

    gt = build_gt_data(gt_detections, gt_objects, gt_sensors)
    ref = build_ref_data(gt_objects, gt_sensors)
    est = build_est_data(gt_detections, gt_sensors)

    data = build_test_vector(description_long='Ideal two object and ideal three detection inside object for each',
                             description_short='Ideal 2 ref, 3 est per each, inside',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt)

    return data


if __name__ == "__main__":
    data_out = get_single_object_with_three_ideal_detection()
