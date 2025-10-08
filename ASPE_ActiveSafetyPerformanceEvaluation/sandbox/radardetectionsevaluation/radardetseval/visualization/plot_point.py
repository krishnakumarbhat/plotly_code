import matplotlib.axes
import numpy as np
import matplotlib.pyplot as plt
from AptivDataExtractors.utilities.MathFunctions import pol2cart


def plot_point(axes: matplotlib.axes.Axes, x, y, vx, vy,
               det_marker='x', v_scale=1.0, **kwargs):
    """
    Plot function for single detection with range rate

    :param axes: axes
    :type axes: matplotlib.axes.Axes
    :param x: position in x-direction
    :param y: position in y-direction
    :param vx: velocity in x-direction
    :param vy: velocity in y-direction
    :param det_marker: marker of point, default 'x'
    :param v_scale: scale of velocity vector used for plotting, default 1.0
    :param kwargs: kwargs passed to plotter
    :return:
    """

    vx_line = [x, x + vx*v_scale]
    vy_line = [y, y + vy*v_scale]

    axes.plot(y, x, marker=det_marker, **kwargs)
    axes.plot(vy_line, vx_line, linestyle="-", **kwargs)


if __name__ == '__main__':
    point_x = 5
    point_y_1 = 3
    point_y_2 = -1
    point_vx = 2
    point_vy = 1
    scale = 0.3
    color = 'r'

    fig1, ax1 = plt.subplots()
    plot_point(ax1, point_x, point_y_1, point_vx, point_vy, color=color, v_scale=scale)
    plot_point(ax1, point_x, point_y_2, point_vx, point_vy, color=color, v_scale=scale)
    plt.grid()
    plt.xlim([-10, 10])
    plt.ylim([0, 10])
    plt.show()

