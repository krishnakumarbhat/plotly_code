import matplotlib.axes
import numpy as np
import matplotlib.pyplot as plt
from aspe.estractors.utilities.MathFunctions import pol2cart, scs2vcs


def plot_sensor(axes: matplotlib.axes.Axes, radar_x=0.0, radar_y=0.0, radar_boresight_angle=0.0,
                azimuth=np.pi/2, max_range=0.0, color='k', **kwargs):

    """
    :param axes: axes
    :type axes: matplotlib.axes.Axes
    :param radar_x:
    :param radar_y:
    :param radar_boresight_angle:
    :param azimuth:
    :param max_range:
    :param color:
    :param kwargs:
    :return:
    """

    # prepare box
    x = np.array([0.05, -0.05, -0.05, 0.05, 0.05])
    y = np.array([0.1, 0.1, -0.1, -0.1, 0.1])

    # prepare FoV
    n_samples = 200
    angles = np.linspace(-azimuth, azimuth, n_samples)
    ranges = np.full_like(angles, max_range)
    x_fov, y_fov = pol2cart(ranges, angles)
    x_fov = np.concatenate([[0], x_fov, [0]])
    y_fov = np.concatenate([[0], y_fov, [0]])

    x_vcs, y_vcs = scs2vcs(x, y, radar_x, radar_y, radar_boresight_angle)
    x_vcs_fov, y_vcs_fov = scs2vcs(x_fov, y_fov, radar_x, radar_y, radar_boresight_angle)

    axes.plot(y_vcs, x_vcs, color=color, linewidth=5, **kwargs)
    axes.plot(y_vcs_fov, x_vcs_fov, color=color, **kwargs)


if __name__ == '__main__':

    fig1, ax1 = plt.subplots()
    plot_sensor(ax1, radar_x=-0.5, radar_y=0.8, radar_boresight_angle=np.deg2rad(40.0),
                azimuth=np.deg2rad(75.0), max_range=90.0)
    plot_sensor(ax1, radar_x=-0.5, radar_y=-0.8, radar_boresight_angle=np.deg2rad(-40.0),
                azimuth=np.deg2rad(75.0), max_range=90.0)
    ax1.axis('equal')