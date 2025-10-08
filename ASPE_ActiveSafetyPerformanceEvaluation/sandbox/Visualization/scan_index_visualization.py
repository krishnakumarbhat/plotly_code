import matplotlib.axes
import pandas
from extractors.Interfaces.IObjects import IObjects

from AptivPerformanceEvaluation.Visualization.extract_scan_index import ScanIndexExtraction
from aspe.estractors.Interfaces.Enums.Object import MovementStatus
from aspe.estractors.Interfaces.IHost import IHost
from aspe.estractors.Interfaces.ExtractedData import ExtractedData
from AptivPerformanceEvaluation.Visualization.plot_object import plot_object
from AptivPerformanceEvaluation.Visualization.plot_detection import plot_detection


def visualize_scan_index(axes: matplotlib.axes.Axes, est_data:ExtractedData, ref_data:ExtractedData,
                         scan_index, f_internal_objects=False, velocity_scale=0.5):
    """
    Visualization of scan index data including reference and estimated data in 2d top view

    :param axes: axes handle
    :type axes: matplotlib.axes.Axes
    :param est_data: estimated data (for all scan indexes)
    :type est_data: ExtractedData
    :param ref_data: reference data (for all scan indexes)
    :type ref_data: ExtractedData
    :param scan_index: scan index for which data should be visualized
    :param f_internal_objects: Flag indicating if internal or reduced object should be used for visualization
    :param velocity_scale: Scale of velocity vectors visualization, default 0.5
    :return:
    """

    # Get data
    si_est_data = ScanIndexExtraction.extract_from_extracted_data(est_data, scan_index)
    si_ref_data = ScanIndexExtraction.extract_from_extracted_data(ref_data, scan_index)

    if f_internal_objects:
        est_object = si_est_data.internal_objects
    else:
        est_object = si_est_data.objects

    # Clear
    axes.clear()

    # Plots
    if si_ref_data.host is not None:
        plot_host_wrapper(axes, si_ref_data.host, label='Host')

    if si_ref_data.objects is not None:
        plot_reference_objects_wrapper(axes, si_ref_data.objects, velocity_scale)

    plot_estimated_objects_wrapper(axes, est_object, velocity_scale)

    if si_est_data.detections is not None:
        plot_detections_wrapper(axes=axes, det_signals=si_est_data.detections.signals, rr_scale=velocity_scale)

    # Plot handling
    legend_handlers, labels, = axes.get_legend_handles_labels()
    unique_legend_handlers, unique_labels, = reduce_labels(legend_handlers, labels)
    axes.grid()
    axes.legend(unique_legend_handlers, unique_labels)
    axes.set_xlabel('Y VCS')
    axes.set_ylabel('X VCS')


def reduce_labels(legend_handlers, labels):
    """
    Reduce duplicated entries in legend

    :param legend_handlers:
    :param labels:
    :return:
    """
    unique_legend_handlers = []
    unique_labels = []
    for legend_handler, label in zip(legend_handlers, labels):
        if label not in unique_labels:
            unique_labels.append(label)
            unique_legend_handlers.append(legend_handler)
    return unique_legend_handlers, unique_labels


def plot_host_wrapper(axes: matplotlib.axes.Axes, host_data_set: IHost, **kwargs):
    """

    :param axes: axes handle
    :type axes: matplotlib.axes.Axes
    :param host_data_set:
    :type host_data_set: IHost
    :param kwargs:
    :return:
    """
    if host_data_set.signals.shape[0] > 0:
        host_first_row = host_data_set.signals.iloc[[0]]

        object_id = ""
        ref_point_x = 0
        ref_point_y = 0
        vx = host_first_row['velocity_otg_x'].values
        vy = host_first_row['velocity_otg_y'].values
        center_x = -host_data_set.bounding_box_dimensions_x * 0.5
        center_y = 0
        orientation = 0
        length = host_data_set.bounding_box_dimensions_x
        width = host_data_set.bounding_box_dimensions_y
        color = 'k'
        plot_object(ax=axes, object_id=object_id, ref_point_x=ref_point_x, ref_point_y=ref_point_y, vx=vx, vy=vy,
                    center_x=center_x, center_y=center_y, orientation=orientation,
                    length=length, width=width, color=color, **kwargs)


def plot_reference_objects_wrapper(axes: matplotlib.axes.Axes, ref_objects: IObjects, velocity_scale=0.5):
    """

    :param axes: axes handle
    :type axes: matplotlib.axes.Axes
    :param ref_objects:
    :type ref_objects: IObjects
    :param velocity_scale:
    :return:
    """

    for index, row in ref_objects.signals.iterrows():
        plot_object_wrapper(axes, row, 'r', velocity_scale=velocity_scale, label='Ref Obj')


def plot_estimated_objects_wrapper(axes: matplotlib.axes.Axes, est_objects: IObjects, velocity_scale=0.5):
    """

    :param axes: axes handle
    :type axes: matplotlib.axes.Axes
    :param est_objects:
    :type est_objects: IObjects
    :param velocity_scale:
    :return:
    """
    for index, row in est_objects.signals.iterrows():
        if row.movement_status == MovementStatus.MOVING:
            plot_object_wrapper(axes, row, 'b', velocity_scale=velocity_scale, label='Moving Obj')
        elif row.movement_status == MovementStatus.STOPPED:
            plot_object_wrapper(axes, row, 'c', velocity_scale=velocity_scale, label='Stopped Obj')
        elif row.movement_status == MovementStatus.STATIONARY:
            plot_object_wrapper(axes, row, 'g', velocity_scale=velocity_scale, label='Stationary Obj')


def plot_object_wrapper(axes: matplotlib.axes.Axes, object_series: pandas.Series,
                        color, velocity_scale=0.5, **kwargs):
    """

    :param axes: axes handle
    :type axes: matplotlib.axes.Axes
    :param object_series: Object series (output of pandas.DataFrame.iterrows()
    :type object_series: pandas.Series
    :param color:
    :param velocity_scale:
    :param kwargs:
    :return:
    """
    object_id = object_series.unique_id
    ref_point_x = object_series.position_x
    ref_point_y = object_series.position_y
    vx = object_series.velocity_otg_x
    vy = object_series.velocity_otg_y
    center_x = object_series.center_x
    center_y = object_series.center_y
    orientation = object_series.bounding_box_orientation
    length = object_series.bounding_box_dimensions_x
    width = object_series.bounding_box_dimensions_y
    plot_object(ax=axes, object_id=object_id, ref_point_x=ref_point_x, ref_point_y=ref_point_y, vx=vx, vy=vy,
                center_x=center_x, center_y=center_y, orientation=orientation,
                length=length, width=width, velocity_scale=velocity_scale, color=color, **kwargs)


def plot_detections_wrapper(axes: matplotlib.axes.Axes, det_signals: pandas.DataFrame, rr_scale=0.5, **kwargs):
    """

    :param axes: axes handle
    :type axes: matplotlib.axes.Axes
    :param det_signals:
    :type det_signals: pandas.DataFrame
    :param rr_scale:
    :param kwargs:
    :return:
    """
    for index, row in det_signals.iterrows():
        x = row.position_x
        y = row.position_y
        range_rate = row.range_rate_comp
        azimuth_vcs = row.azimuth_vcs
        color = 'm'
        plot_detection(axes=axes, x=x, y=y, range_rate=range_rate, azimuth_vcs=azimuth_vcs, rr_scale=rr_scale,
                       label='Dets', color=color)
