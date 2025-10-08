import matplotlib.axes
import numpy as np
import matplotlib.pyplot as plt
from extractors.utilities.MathFunctions import pol2cart

from radardetseval.Iface.PolyLineDS import PolyLineDS
from radardetseval.visualization.plot_sensor import plot_sensor


def plot_polyline(axes: matplotlib.axes.Axes, polyline: PolyLineDS,
                  vertex_marker='x', vel_scale=1.0, color='b', **kwargs):
    """

    :param axes:
    :param polyline:
    :param vertex_marker:
    :param vel_scale:
    :param color:
    :param kwargs:
    :return:
    """

    x = polyline.signals.x
    y = polyline.signals.y
    axes.plot(y, x, marker=vertex_marker, color=color, **kwargs)
    for i, row in polyline.signals.iterrows():
        vx_line = [row['x'], row['x'] + row['vx']*vel_scale]
        vy_line = [row['y'], row['y'] + row['vy']*vel_scale]
        axes.plot(vy_line, vx_line, linestyle="-", linewidth=3, color=color, **kwargs)


def plot_polyline_range_rates(axes: matplotlib.axes.Axes, polyline: PolyLineDS,
                              rr_scale=1.0, color='r', **kwargs):
    """

    :param axes:
    :param polyline:
    :param rr_scale:
    :param color:
    :param kwargs:
    :return:
    """

    for i, row in polyline.signals.iterrows():
        x = row['x']
        y = row['y']
        azimuth = np.arctan2(y, x)
        range_rate = row['vx'] * np.cos(azimuth) + row['vy'] * np.sin(azimuth)

        d_vx, d_vy = pol2cart(range_rate * rr_scale, azimuth)
        vx_line = [x, x + d_vx]
        vy_line = [y, y + d_vy]

        axes.plot(vy_line, vx_line, linestyle="-", linewidth=3, color=color, **kwargs)


if __name__ == '__main__':
    gt_polyline = PolyLineDS()
    gt_polyline.add_point(5, 7, 3, 4)
    gt_polyline.add_point(5, -4, 3, 4)
    gt_polyline.add_point(2, -6, 3, 4)

    fig1, ax1 = plt.subplots()

    plot_sensor(ax1, azimuth=np.deg2rad(75.0), max_range=15.0)
    plot_polyline(ax1, gt_polyline)
    plot_polyline_range_rates(ax1, gt_polyline)
    ax1.axis('equal')

    fig2, ax2 = plt.subplots()
    plot_sensor(ax2, azimuth=np.deg2rad(75.0), max_range=15.0)
    plot_polyline(ax2, gt_polyline)
    gt_polyline.signals = gt_polyline.discretize_single_polygon(gt_polyline.signals, 0.2)
    plot_polyline_range_rates(ax2, gt_polyline)
    ax2.axis('equal')


