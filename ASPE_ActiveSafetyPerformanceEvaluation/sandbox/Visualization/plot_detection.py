import matplotlib.axes
import numpy as np
import matplotlib.pyplot as plt
from aspe.estractors.utilities.MathFunctions import pol2cart


def plot_detection(axes: matplotlib.axes.Axes, x, y, range_rate, azimuth_vcs, det_id=None, det_marker='x', rr_scale=0.5, **kwargs):
    """
    Plot function for single detection with range rate

    :param axes: axes
    :type axes: matplotlib.axes.Axes
    :param x: position of detection in x coordinate
    :param y: position of detection in y coordinate
    :param range_rate: range rate of detection
    :param azimuth_vcs: azimuth aligned with VCS
    :param det_id: ID of detection, default None
    :param det_marker: marker of detection, default 'x'
    :param rr_scale: scale of range rate vector used for plotting, default 0.5
    :param kwargs:
    :return:
    """
    plot_det_ref_point(axes, x, y, det_id, det_marker, **kwargs)
    plot_det_range_rate(axes, x, y, range_rate, azimuth_vcs, rr_scale=rr_scale, **kwargs)


def plot_det_ref_point(axes: matplotlib.axes.Axes, x, y, det_id=None, det_marker='x', **kwargs):
    """

    :param axes: axes handle
    :type axes: matplotlib.axes.Axes
    :param x: position of detection in x coordinate
    :param y: position of detection in y coordinate
    :param det_id: ID of detection, default None
    :param det_marker: marker of detection, default 'x'
    :param kwargs:
    :return:
    """

    axes.plot(y, x, marker=det_marker, **kwargs)
    if det_id is not None:
        axes.annotate(str(int(det_id)), (y, x), **kwargs)


def plot_det_range_rate(axes: matplotlib.axes.Axes, x, y, range_rate, azimuth, rr_scale=0.5, **kwargs):
    """

    :param axes: axes handle
    :type axes: matplotlib.axes.Axes
    :param x: position of detection in x coordinate
    :param y: position of detection in y coordinate
    :param range_rate: range rate of detection
    :param azimuth_vcs: azimuth aligned with VCS
    :param rr_scale: scale of range rate vector used for plotting, default 0.5
    :param kwargs:
    :return:
    """
    vx, vy = pol2cart(range_rate*rr_scale, azimuth)
    x_line = [x, x + vx]
    y_line = [y, y + vy]
    axes.plot(y_line, x_line, linestyle="-", **kwargs)


if __name__ == '__main__':
    x = 5
    y = 5.5
    azimuth_vcs = np.deg2rad(45.0)
    range_rate = 3
    scale = 0.25
    color = 'r'

    fig1, ax1 = plt.subplots()
    plot_detection(ax1, x, y, range_rate, azimuth_vcs, color=color)
    plt.grid()
    plt.show()

    fig2, ax2 = plt.subplots()
    plot_det_ref_point(ax2, x, y, color=color)
    plot_det_range_rate(ax2, x, y, range_rate, azimuth_vcs, color=color)

    plt.grid()
    plt.show()