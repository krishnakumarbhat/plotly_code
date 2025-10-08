import numpy as np
import pandas as pd

from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors import default_data
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.support_functions import \
    get_default_sensors, build_gt_data, build_ref_data, build_est_data, build_test_vector, get_detection_on_state, \
    get_multi_bounce_detection, get_wheel_spin_detection
from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.tv_visualization import \
    visualize_test_vector


def get_two_objects_with_range_rate_conflicts():

    gt_sensors = get_default_sensors()
    gt_object1 = default_data.default_object.copy()
    gt_object1.update(pd.Series({
        'position_x': 10.0,
        'position_y': 0.0,
        'velocity_otg_x': 5.0,
        'velocity_otg_y': 0.0,
        'bounding_box_dimensions_x': 4.0,
        'bounding_box_dimensions_y': 1.8,
        'center_x': 10.0,
        'center_y': 0.0,
    }))

    gt_object2 = default_data.default_object.copy()
    gt_object2.update(pd.Series({
        'position_x': 9.5,
        'position_y': 3.0,
        'velocity_otg_x': 5.0,
        'velocity_otg_y': 0.0,
        'bounding_box_dimensions_x': 4.0,
        'bounding_box_dimensions_y': 1.8,
        'center_x': 9.5,
        'center_y': 3.0,
    }))
    gt_objects = [gt_object1, gt_object2]

    gt_detection1 = get_detection_on_state(gt_object1, gt_sensors, 0.0, 1.0, 1)
    gt_detection2 = get_detection_on_state(gt_object2, gt_sensors, 0.0, 0.0, 1)
    est_detection1 = pd.DataFrame([gt_detection1, gt_detection2]).mean()
    gt_detections = [gt_detection1, gt_detection2]
    est_detections = [est_detection1]

    gt = build_gt_data(gt_detections, gt_objects, gt_sensors)
    ref = build_ref_data(gt_objects, gt_sensors)
    est = build_est_data(est_detections, gt_sensors)

    expectation = {
        'n_associated': 0
    }

    data = build_test_vector(description_long='Two close objects with one detection in between',
                             description_short='2 ref, 1 est between',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt,
                             expectation=expectation)

    return data


def get_three_objects_with_different_features():

    gt_sensors = get_default_sensors()
    gt_object0 = default_data.default_object.copy()
    gt_object0.update(pd.Series({
        'position_x': 9.0,
        'position_y': 0.0,
        'velocity_otg_x': 5.0,
        'velocity_otg_y': 0.0,
        'bounding_box_dimensions_x': 4.0,
        'bounding_box_dimensions_y': 1.8,
        'center_x': 9.0,
        'center_y': 0.0,
    }))

    gt_object1 = default_data.default_object.copy()
    gt_object1.update(pd.Series({
        'position_x': 7.0,
        'position_y': 3.0,
        'velocity_otg_x': 5.0,
        'velocity_otg_y': 0.0,
        'bounding_box_dimensions_x': 11.0,
        'bounding_box_dimensions_y': 2.2,
        'center_x': 7.0,
        'center_y': 3.0,
    }))

    gt_object2 = default_data.default_object.copy()
    gt_object2.update(pd.Series({
        'position_x': 11,
        'position_y': -2.5,
        'velocity_otg_x': -5.0,
        'velocity_otg_y': 0.0,
        'bounding_box_orientation': np.deg2rad(180.0),
        'bounding_box_dimensions_x': 5.0,
        'bounding_box_dimensions_y': 1.8,
        'center_x': 11.0,
        'center_y': -2.5,
    }))
    gt_objects = [gt_object0, gt_object1, gt_object2]

    ref_object0 = gt_object0.copy()
    ref_object0['position_x'] += 0.1
    ref_object0['position_y'] -= 0.1
    ref_object0['center_x'] += 0.1
    ref_object0['center_y'] -= 0.1
    ref_object0['velocity_otg_x'] -= 0.1
    ref_object0['velocity_otg_y'] += 0.2
    ref_object0['bounding_box_orientation'] += np.deg2rad(1.0)

    ref_object1 = gt_object1.copy()
    ref_object1['position_x'] -= 0.05
    ref_object1['position_y'] -= 0.1
    ref_object1['center_x'] -= 0.05
    ref_object1['center_y'] -= 0.1
    ref_object1['velocity_otg_x'] += 0.1
    ref_object1['velocity_otg_y'] += 0.05
    ref_object1['bounding_box_orientation'] += np.deg2rad(0.2)

    ref_object2 = gt_object2.copy()
    ref_object2['position_x'] -= 0.1
    ref_object2['position_y'] -= 0.1
    ref_object2['center_x'] -= 0.1
    ref_object2['center_y'] -= 0.1
    ref_object2['velocity_otg_x'] -= 0.05
    ref_object2['velocity_otg_y'] += 0.15
    ref_object2['bounding_box_orientation'] -= np.deg2rad(1.0)

    ref_objects = [ref_object0, ref_object1, ref_object2]

    gt_detection0 = get_detection_on_state(gt_object0, gt_sensors, 0.0, 0.5, 1)
    est_detection0 = gt_detection0.copy()

    gt_detection1 = get_detection_on_state(gt_object0, gt_sensors, 0.0, 1.0, 1)
    gt_detection2 = get_detection_on_state(gt_object1, gt_sensors, 0.5, 0.0, 1)
    est_detection1 = pd.DataFrame([gt_detection1, gt_detection2]).mean()

    gt_detection3 = get_detection_on_state(gt_object0, gt_sensors, 0.0, 0.0, 1)
    est_detection2 = gt_detection3.copy()
    est_detection2['azimuth'] -= np.deg2rad(0.5)

    gt_detection4 = get_detection_on_state(gt_object1, gt_sensors, 0.0, 0.0, 1)
    est_detection3 = gt_detection4.copy()
    est_detection3['azimuth'] += np.deg2rad(1.0)
    est_detection3['range_rate'] += 0.05
    est_detection4 = get_multi_bounce_detection(gt_object1, gt_sensors, 0.0, 0.0, 1, 2.0)

    gt_detection5 = get_detection_on_state(gt_object1, gt_sensors, 0.2, 0.00, 1)
    est_detection5 = get_wheel_spin_detection(gt_object1, gt_sensors, 0.2, 0.05, 1, 2.0)
    est_detection6 = get_wheel_spin_detection(gt_object1, gt_sensors, 0.21, 0.03, 1, 0.5)
    est_detection7 = get_wheel_spin_detection(gt_object1, gt_sensors, 0.18, 0.04, 1, 1.5)
    est_detection8 = get_multi_bounce_detection(gt_object1, gt_sensors, 0.2, 0.0, 1, 2.0)

    gt_detection6 = get_detection_on_state(gt_object1, gt_sensors, 0.0, 0.5, 1)
    est_detection9 = gt_detection6.copy()
    est_detection9['range'] -= 0.1
    est_detection9['range_rate'] += 0.05

    gt_detection7 = get_detection_on_state(gt_object1, gt_sensors, 0.0, 1.0, 1)
    est_detection10 = gt_detection7.copy()
    est_detection10['range'] -= 0.1
    est_detection10['azimuth'] += np.deg2rad(1.0)

    gt_detection8 = get_detection_on_state(gt_object1, gt_sensors, 0.35, 0.0, 1)
    est_detection11 = gt_detection8.copy()
    est_detection11['range'] += 0.1
    est_detection11['range_rate'] -= 0.05

    gt_detection9 = get_detection_on_state(gt_object1, gt_sensors, 0.65, 0.0, 1)
    est_detection12 = gt_detection9.copy()
    est_detection12['range'] += 0.1
    est_detection12['range_rate'] += 0.05
    est_detection12['azimuth'] += np.deg2rad(1.0)
    est_detection13 = get_wheel_spin_detection(gt_object1, gt_sensors, 0.7, 0.00, 1, 1.3)
    est_detection13['azimuth'] -= np.deg2rad(1.0)

    est_detection14 = default_data.default_detection.copy()
    est_detection14['range'] = 3.0
    est_detection14['azimuth'] = np.deg2rad(-28.0)
    est_detection14['range_rate'] = 2.5

    gt_detection10 = get_detection_on_state(gt_object2, gt_sensors, 1.0, 1.0, 1)
    est_detection15 = gt_detection10.copy()
    est_detection15['range'] += 0.02
    est_detection15['range_rate'] -= 0.05
    est_detection15['azimuth'] -= np.deg2rad(6.0)

    gt_detection11 = get_detection_on_state(gt_object2, gt_sensors, 1.0, 0.0, 1)
    est_detection16 = gt_detection11.copy()
    est_detection16['range'] += 0.2
    est_detection16['range_rate'] -= 0.01
    est_detection16['azimuth'] += np.deg2rad(1.2)

    gt_detection12 = get_detection_on_state(gt_object2, gt_sensors, 0.7, 0.2, 1)
    est_detection17 = gt_detection12.copy()
    est_detection17['range'] += 0.1
    est_detection17['range_rate'] -= 0.05
    est_detection17['azimuth'] -= np.deg2rad(0.2)

    gt_detections = [gt_detection0, gt_detection1, gt_detection2, gt_detection3, gt_detection4, gt_detection5,
                     gt_detection6, gt_detection7, gt_detection8, gt_detection9, gt_detection10, gt_detection11,
                     gt_detection12]
    est_detections = [est_detection0, est_detection1, est_detection2, est_detection3, est_detection4, est_detection5,
                      est_detection6, est_detection7, est_detection8, est_detection9, est_detection10, est_detection11,
                      est_detection12, est_detection13, est_detection14, est_detection15, est_detection16,
                      est_detection17]

    gt = build_gt_data(gt_detections, gt_objects, gt_sensors)
    ref = build_ref_data(ref_objects, gt_sensors)
    est = build_est_data(est_detections, gt_sensors)

    expectation = {
        'n_associated': 9,
        'associated_pairs': np.array([[0, 0],
                                      [0, 2],
                                      [1, 3],
                                      [1, 9],
                                      [1, 10],
                                      [1, 11],
                                      [1, 12],
                                      [2, 16],
                                      [2, 17]]),
        'est_ids_true_positives': np.array([0, 2, 3, 9, 10, 11, 12, 16, 17]),
        'est_ids_wheel_spins': np.array([5, 6, 7, 13]),
        'est_ids_double_bounce': np.array([4, 8]),
        'est_ids_clutter': np.array([14]),
        'est_ids_azimuth_error': np.array([1, 15]),
    }

    data = build_test_vector(description_long='Three objects, many different detections',
                             description_short='3 ref, many est',
                             reference_data=ref,
                             estimated_data=est,
                             ground_truth_data=gt,
                             expectation=expectation)

    return data


if __name__ == "__main__":
    data_out = get_three_objects_with_different_features()
    visualize_test_vector(data_out, f_show_gt=False)
