import numpy as np
import pandas as pd

from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors import default_data
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.support_functions import \
    get_default_sensors, build_gt_data, build_ref_data, build_est_data, build_test_vector, get_detection_on_state, \
    get_multi_bounce_detection, get_wheel_spin_detection
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.tv_visualization import \
    visualize_test_vector


def get_angle_jumps_from_overtaking_object():

    gt_sensors = get_default_sensors()
    gt_sensors.per_sensor = default_data.front_right_sensor_per_sensor.copy()
    gt_sensors.signals['sensor_id'] = default_data.front_right_sensor_id
    gt_sensors.per_look['sensor_id'] = default_data.front_right_sensor_id
    gt_sensors.signals.update(pd.DataFrame([[25.0, 0.0]], columns=['velocity_otg_x', 'velocity_otg_y']))

    gt_object0 = default_data.default_object.copy()
    gt_object0.update(pd.Series({
        'position_x': 2.0,
        'position_y': 3.0,
        'velocity_otg_x': 20.0,
        'velocity_otg_y': 0.0,
        'bounding_box_dimensions_x': 10.0,
        'bounding_box_dimensions_y': 2.2,
        'center_x': 2.0,
        'center_y': 3.0,
    }))

    gt_objects = [gt_object0]

    gt_detection0 = get_detection_on_state(gt_object0, gt_sensors, 0.2, 0.0, default_data.front_right_sensor_id)
    gt_detection1 = get_detection_on_state(gt_object0, gt_sensors, 0.25, 0.0, default_data.front_right_sensor_id)
    gt_detection2 = get_detection_on_state(gt_object0, gt_sensors, 0.275, 0.0, default_data.front_right_sensor_id)

    est_detection0 = gt_detection0.copy()
    est_detection0['azimuth'] += np.deg2rad(-22.5)
    est_detection1 = gt_detection1.copy()
    est_detection1['azimuth'] += np.deg2rad(-60.0)
    est_detection2 = gt_detection2.copy()
    est_detection2['azimuth'] += np.deg2rad(-30.0)

    gt_detections = [gt_detection0, gt_detection1, gt_detection2]
    est_detections = [est_detection0, est_detection1, est_detection2]

    gt = build_gt_data(gt_detections, gt_objects, gt_sensors)
    ref = build_ref_data(gt_objects, gt_sensors)
    est = build_est_data(est_detections, gt_sensors)

    expectation = {
        'n_associated': 0,
        'est_ids_azimuth_error': np.array([0, 1, 2])
    }

    data = build_test_vector(description_long='Host overtaking target, 3 angle jumps',
                             description_short='Host overtaking, 3 AJ',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt,
                             expectation=expectation)

    return data


def get_angle_jumps_small_rel_speed_object():

    gt_sensors = get_default_sensors()
    gt_sensors.per_sensor = default_data.front_right_sensor_per_sensor.copy()
    gt_sensors.signals['sensor_id'] = default_data.front_right_sensor_id
    gt_sensors.per_look['sensor_id'] = default_data.front_right_sensor_id
    gt_sensors.signals.update(pd.DataFrame([[21.0, 0.0]], columns=['velocity_otg_x', 'velocity_otg_y']))

    gt_object0 = default_data.default_object.copy()
    gt_object0.update(pd.Series({
        'position_x': 2.0,
        'position_y': 3.0,
        'velocity_otg_x': 20.0,
        'velocity_otg_y': 0.0,
        'bounding_box_dimensions_x': 10.0,
        'bounding_box_dimensions_y': 2.2,
        'center_x': 2.0,
        'center_y': 3.0,
    }))

    gt_objects = [gt_object0]

    gt_detection0 = get_detection_on_state(gt_object0, gt_sensors, 0.2, 0.0, default_data.front_right_sensor_id)
    gt_detection1 = get_detection_on_state(gt_object0, gt_sensors, 0.25, 0.0, default_data.front_right_sensor_id)
    gt_detection2 = get_detection_on_state(gt_object0, gt_sensors, 0.275, 0.0, default_data.front_right_sensor_id)

    est_detection0 = gt_detection0.copy()
    est_detection0['azimuth'] += np.deg2rad(-22.5)
    est_detection1 = gt_detection1.copy()
    est_detection1['azimuth'] += np.deg2rad(-60.0)
    est_detection2 = gt_detection2.copy()
    est_detection2['azimuth'] += np.deg2rad(-30.0)

    gt_detections = [gt_detection0, gt_detection1, gt_detection2]
    est_detections = [est_detection0, est_detection1, est_detection2]

    gt = build_gt_data(gt_detections, gt_objects, gt_sensors)
    ref = build_ref_data(gt_objects, gt_sensors)
    est = build_est_data(est_detections, gt_sensors)

    expectation = {
        'n_associated': 1,
        'associated_pairs': np.array([[0, 0]]),
        'est_ids_true_positives': np.array([0]), # Not enough information to reject this detections
        'est_ids_azimuth_error': np.array([1, 2])
    }

    data = build_test_vector(description_long='Host overtaking target slowly, 3 angle jumps',
                             description_short='Host overtaking slowly, 3 AJ',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt,
                             expectation=expectation)

    return data


def get_angle_jumps_same_speed_object():

    gt_sensors = get_default_sensors()
    gt_sensors.per_sensor = default_data.front_right_sensor_per_sensor.copy()
    gt_sensors.signals['sensor_id'] = default_data.front_right_sensor_id
    gt_sensors.per_look['sensor_id'] = default_data.front_right_sensor_id
    gt_sensors.signals.update(pd.DataFrame([[20.0, 0.0]], columns=['velocity_otg_x', 'velocity_otg_y']))

    gt_object0 = default_data.default_object.copy()
    gt_object0.update(pd.Series({
        'position_x': 2.0,
        'position_y': 3.0,
        'velocity_otg_x': 20.0,
        'velocity_otg_y': 0.0,
        'bounding_box_dimensions_x': 10.0,
        'bounding_box_dimensions_y': 2.2,
        'center_x': 2.0,
        'center_y': 3.0,
    }))

    gt_objects = [gt_object0]

    gt_detection0 = get_detection_on_state(gt_object0, gt_sensors, 0.2, 0.0, default_data.front_right_sensor_id)
    gt_detection1 = get_detection_on_state(gt_object0, gt_sensors, 0.25, 0.0, default_data.front_right_sensor_id)
    gt_detection2 = get_detection_on_state(gt_object0, gt_sensors, 0.275, 0.0, default_data.front_right_sensor_id)

    est_detection0 = gt_detection0.copy()
    est_detection0['azimuth'] += np.deg2rad(-22.5)
    est_detection1 = gt_detection1.copy()
    est_detection1['azimuth'] += np.deg2rad(-60.0)
    est_detection2 = gt_detection2.copy()
    est_detection2['azimuth'] += np.deg2rad(-30.0)

    gt_detections = [gt_detection0, gt_detection1, gt_detection2]
    est_detections = [est_detection0, est_detection1, est_detection2]

    gt = build_gt_data(gt_detections, gt_objects, gt_sensors)
    ref = build_ref_data(gt_objects, gt_sensors)
    est = build_est_data(est_detections, gt_sensors)

    expectation = {
        'n_associated': 2,
        'associated_pairs': np.array([[0, 0],
                                      [0, 2]]),
        'est_ids_true_positives': np.array([0, 2]),  # Not enough information to reject these detections
        'est_ids_azimuth_error': np.array([1])
    }

    data = build_test_vector(description_long='Host parallel to target, 3 angle jumps',
                             description_short='Target parallel, 3 AJ',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt,
                             expectation=expectation)

    return data


if __name__ == "__main__":
    data_out = get_angle_jumps_same_speed_object()
    visualize_test_vector(data_out, f_show_gt=True, f_show_expected_label=True)
