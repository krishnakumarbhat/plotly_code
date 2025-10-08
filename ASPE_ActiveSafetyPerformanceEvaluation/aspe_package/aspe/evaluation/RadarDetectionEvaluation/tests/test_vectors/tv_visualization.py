import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

from aspe.evaluation.RadarDetectionEvaluation.tests.test_vectors.ideal_data import *
from aspe.evaluation.Visualization.plot_detection import plot_detection
from aspe.evaluation.Visualization.plot_object import plot_object, plot_bb_ref_point_vel_cart
from aspe.evaluation.Visualization.plot_sensor import plot_sensor
from aspe.evaluation.Visualization.utilities import publish_reduced_labels


def visualize_test_vector(data,
                          f_show_gt=True,
                          f_show_ref=True,
                          f_show_est=True,
                          f_show_expected_label=False):
    ax = plt.subplot(1, 1, 1)
    gt = data['ground_truth_data']
    ref = data['reference_data']
    est = data['estimated_data']
    expectation = data['expectation']
    if f_show_gt:
        plot_gt_objects(ax, gt.objects.signals)
        plot_gt_detections(ax, gt.detections.signals)
    plot_gt_sensors(ax, gt.sensors)
    if f_show_ref:
        plot_ref_objects(ax, ref.objects.signals)
    if f_show_est:
        if f_show_expected_label:
            plot_est_detections_with_labels(ax, est.detections.signals, expectation)
        else:
            plot_est_detections(ax, est.detections.signals)
    publish_reduced_labels(ax)
    ax.set_xlabel('y [m]')
    ax.set_ylabel('x [m]')
    ax.set_title(data['description_long'])
    ax.grid()
    ax.axis('equal')


def plot_gt_objects(ax, objects_signals):
    for idx, row in objects_signals.iterrows():
        plot_object(ax, None, row['position_x'], row['position_y'],
                    row['velocity_otg_x'], row['velocity_otg_y'],
                    row['center_x'], row['center_y'],
                    row['bounding_box_dimensions_x'], row['bounding_box_dimensions_y'], row['bounding_box_orientation'],
                    label='Ground Truth', color='k')


def plot_ref_objects(ax, objects_signals):
    for idx, row in objects_signals.iterrows():
        plot_object(ax, row['unique_id'], row['position_x'], row['position_y'],
                    row['velocity_otg_x'], row['velocity_otg_y'],
                    row['center_x'], row['center_y'],
                    row['bounding_box_dimensions_x'], row['bounding_box_dimensions_y'], row['bounding_box_orientation'],
                    label='Reference', color='r')


def plot_gt_detections(ax, detections_signals):
    for idx, row in detections_signals.iterrows():
        plot_detection(ax, row['position_x'], row['position_y'],
                       row['range_rate'], row['azimuth_vcs'], det_id=row['unique_id'],
                       label='Ground Truth', color='k')


def plot_est_detections(ax, detections_signals):
    for idx, row in detections_signals.iterrows():
        plot_detection(ax, row['position_x'], row['position_y'],
                       row['range_rate'], row['azimuth_vcs'], det_id=row['unique_id'],
                       label='Measurement', color='b')


def plot_est_detections_with_labels(ax, detections_signals, expected_data):
    expected_data_keys = expected_data.keys()
    if 'est_ids_true_positives' in expected_data_keys:
        plot_tp_detections(ax, detections_signals, expected_data['est_ids_true_positives'])
    if 'est_ids_wheel_spins' in expected_data_keys:
        plot_wheel_spin_detections(ax, detections_signals, expected_data['est_ids_wheel_spins'])
    if 'est_ids_double_bounce' in expected_data_keys:
        plot_double_bounce_detections(ax, detections_signals, expected_data['est_ids_double_bounce'])
    if 'est_ids_azimuth_error' in expected_data_keys:
        plot_azimuth_error_detections(ax, detections_signals, expected_data['est_ids_azimuth_error'])
    if 'est_ids_clutter' in expected_data_keys:
        plot_clutter_detections(ax, detections_signals, expected_data['est_ids_clutter'])


def plot_tp_detections(ax, detections_signals, ids):
    for single_id in ids:
        row = detections_signals.iloc[single_id, :]
        plot_detection(ax, row['position_x'], row['position_y'],
                       row['range_rate'], row['azimuth_vcs'],
                       label='True Positive', color=mcolors.TABLEAU_COLORS['tab:blue'])


def plot_wheel_spin_detections(ax, detections_signals, ids):
    for single_id in ids:
        row = detections_signals.iloc[single_id, :]
        plot_detection(ax, row['position_x'], row['position_y'],
                       row['range_rate'], row['azimuth_vcs'],
                       label='Wheel Spin', color=mcolors.TABLEAU_COLORS['tab:orange'])


def plot_double_bounce_detections(ax, detections_signals, ids):
    for single_id in ids:
        row = detections_signals.iloc[single_id, :]
        plot_detection(ax, row['position_x'], row['position_y'],
                       row['range_rate'], row['azimuth_vcs'],
                       label='Double Bounce', color=mcolors.TABLEAU_COLORS['tab:brown'])


def plot_azimuth_error_detections(ax, detections_signals, ids):
    for single_id in ids:
        row = detections_signals.iloc[single_id, :]
        plot_detection(ax, row['position_x'], row['position_y'],
                       row['range_rate'], row['azimuth_vcs'],
                       label='Azimuth Error', color=mcolors.TABLEAU_COLORS['tab:purple'])


def plot_clutter_detections(ax, detections_signals, ids):
    for single_id in ids:
        row = detections_signals.iloc[single_id, :]
        plot_detection(ax, row['position_x'], row['position_y'],
                       row['range_rate'], row['azimuth_vcs'],
                       label='Clutter', color=mcolors.TABLEAU_COLORS['tab:olive'])


def plot_gt_sensors(ax, sensor_ds):
    signals = sensor_ds.per_sensor.merge(sensor_ds.per_look, on='sensor_id')
    signals = signals.merge(sensor_ds.signals, on='sensor_id')
    for idx, row in signals.iterrows():
        plot_sensor(ax, row['position_x'], row['position_y'], row['boresight_az_angle'],
                    row['max_azimuth'], row['max_range'], color='k')
        plot_bb_ref_point_vel_cart(ax, row['position_x'], row['position_y'],
                                   row['velocity_otg_x'], row['velocity_otg_y'], color='k')


if __name__ == "__main__":
    single_data = get_two_object_with_three_ideal_detection_inside_each()
    visualize_test_vector(single_data)

