import matplotlib.axes
import numpy as np
import matplotlib.pyplot as plt
from AptivDataExtractors.utilities.MathFunctions import pol2cart


def plot_detection_scs(axes: matplotlib.axes.Axes, det_range, det_range_rate, det_azimuth,
                       det_marker='x', rr_scale=1.0, **kwargs):
    """
    Plot function for single detection with range rate

    :param axes: axes
    :type axes: matplotlib.axes.Axes
    :param det_range: distance to sensor of detection
    :param det_range_rate: range rate of detection
    :param det_azimuth: azimuth of detections (positive right side)
    :param det_marker: marker of detection, default 'x'
    :param rr_scale: scale of range rate vector used for plotting, default 0.5
    :param kwargs: kwargs passed to plotter
    :return:
    """

    x,y = pol2cart(det_range, det_azimuth)
    d_vx, d_vy = pol2cart(det_range_rate * rr_scale, det_azimuth)

    vx_line = [x, x + d_vx]
    vy_line = [y, y + d_vy]

    axes.plot(y, x, marker=det_marker, **kwargs)
    axes.plot(vy_line, vx_line, linestyle="-", **kwargs)


if __name__ == '__main__':
    det_range_common = 4
    det_azimuth_1 = np.deg2rad(45.0)
    det_azimuth_2 = np.deg2rad(32.0)
    det_range_rate_common = -2
    scale = 0.3
    color = 'r'

    fig1, ax1 = plt.subplots()
    plot_detection_scs(ax1, det_range_common, det_range_rate_common, det_azimuth_1, color=color, rr_scale=scale)
    plot_detection_scs(ax1, det_range_common, det_range_rate_common, det_azimuth_2, color=color, rr_scale=scale)
    plt.grid()
    plt.xlim([-10, 10])
    plt.ylim([0, 10])
    plt.show()

