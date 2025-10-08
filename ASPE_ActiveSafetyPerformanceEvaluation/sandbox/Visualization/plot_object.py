import matplotlib
import numpy as np
import matplotlib.pyplot as plt
from aspe.estractors.utilities.MathFunctions import lcs2vcs, pol2cart


def plot_object(ax: matplotlib.axes.Axes, object_id, ref_point_x, ref_point_y, vx, vy, center_x, center_y,
                length, width, orientation, velocity_scale=0.5, **kwargs):
    """
    Plot function for single object: oriented bounding box with ID, reference point and velocity

    :param ax: axes handle
    :type ax: matplotlib.axes.Axes
    :param object_id:
    :param ref_point_x: position of reference point in x coordinate
    :param ref_point_y: position of reference point in y coordinate
    :param vx: velocity of reference point in x coordinate
    :param vy: velocity of reference point in y coordinate
    :param center_x: position of center point in x coordinate
    :param center_y: position of center point in x coordinate
    :param length: length of an object
    :param width: width of an object
    :param orientation: orientation (or yaw angle) of an object
    :param velocity_scale: scale of velocity vector used for plotting, default 0.5
    :param kwargs:
    :return:
    """
    plot_bb(ax, center_x, center_y, length, width, orientation, **kwargs)
    plot_bb_ref_point(ax, ref_point_x, ref_point_y, object_id, **kwargs)
    plot_bb_ref_point_vel_cart(ax, ref_point_x, ref_point_y, vx, vy, scale=velocity_scale, **kwargs)


def plot_bb(ax: matplotlib.axes.Axes, center_x, center_y, length, width, orientation, **kwargs):
    """
    Plot function for single Bounding Box (BB)

    :param ax: axes handle
    :type ax: matplotlib.axes.Axes
    :param center_x: position of center point in x coordinate
    :param center_y: position of center point in x coordinate
    :param length: length of an BB
    :param width: width of an BB
    :param orientation: orientation (or yaw angle) of an BB
    :param kwargs:
    :return:
    """

    # Points starting from left front
    points_x_tcs = np.array([length, length, -length, -length, length]) * 0.5
    points_y_tcs = np.array([-width, width, width, -width, -width]) * 0.5
    points_x_vcs, points_y_vcs = lcs2vcs(points_x_tcs, points_y_tcs, center_x, center_y, orientation)
    ax.plot(points_y_vcs, points_x_vcs, linestyle='-', **kwargs)


def plot_bb_orientation(ax: matplotlib.axes.Axes, center_x, center_y, length, width, orientation, **kwargs):
    """
    Plot function for single Bounding Box (BB) orientation indicator

    :param ax: axes handle
    :type ax: matplotlib.axes.Axes
    :param center_x: position of center point in x coordinate
    :param center_y: position of center point in x coordinate
    :param length: length of an BB
    :param width: width of an BB
    :param orientation: orientation (or yaw angle) of an BB
    :param kwargs:
    :return:
    """
    # Points starting from left front
    points_x_tcs = np.array([length, length*0.5, length*0.5, length]) * 0.5
    points_y_tcs = np.array([0, width, -width, 0]) * 0.5
    points_x_vcs, points_y_vcs = lcs2vcs(points_x_tcs, points_y_tcs, center_x, center_y, orientation)
    ax.plot(points_y_vcs, points_x_vcs, linestyle=':', **kwargs)


def plot_bb_ref_point(ax: matplotlib.axes.Axes, x, y, object_id=None, **kwargs):
    """
    Plot of reference point with optional object ID plotting

    :param ax: axes handle
    :type ax: matplotlib.axes.Axes
    :param x: position of reference point in x coordinate
    :param y: position of reference point in y coordinate
    :param object_id: Object ID
    :param kwargs:
    :return:
    """
    ax.plot(y, x, marker="^", **kwargs)
    if object_id is not None:
        ax.annotate(str(int(object_id)), (y, x), **kwargs)


def plot_bb_ref_point_vel_cart(ax: matplotlib.axes.Axes, x, y, vx, vy, scale=0.5, **kwargs):
    """
    Plot of velocity vector for given point by cartesian coordinate

    :param ax: axes handle
    :type ax: matplotlib.axes.Axes
    :param x: position of reference point in x coordinate
    :param y: position of reference point in y coordinate
    :param vx: velocity of reference point in x coordinate
    :param vy: velocity of reference point in y coordinate
    :param scale: scale of velocity vector used for plotting, default 0.5
    :param kwargs:
    :return:
    """
    x_line = [x, x + vx*scale]
    y_line = [y, y + vy*scale]
    ax.plot(y_line, x_line, linestyle="-", linewidth=2, **kwargs)


def plot_bb_ref_point_vel_polar(ax: matplotlib.axes.Axes, x, y, magnitude, heading, scale=0.5, **kwargs):
    """
    Plot of velocity vector for given point by polar coordinate

    :param ax: axes handle
    :type ax: matplotlib.axes.Axes
    :param x: position of reference point in x coordinate
    :param y: position of reference point in y coordinate
    :param magnitude: velocity magnitude
    :param heading: velocity heading (angle)
    :param scale: scale of velocity vector used for plotting, default 0.5
    :param kwargs:
    :return:
    """
    vx, vy = pol2cart(magnitude*scale, heading)
    x_line = [x, x + vx]
    y_line = [y, y + vy]
    ax.plot(y_line, x_line, linestyle="-", **kwargs)


if __name__ == '__main__':
    object_id = 3
    ref_point_x = 6
    ref_point_y = 5
    vx = 5
    vy = 5.5
    center_x = 5
    center_y = 4
    orientation = np.deg2rad(45.0)
    length = 5
    width = 2
    color = 'r'
    transparency = 0.2

    fig1, ax1 = plt.subplots()
    plot_object(ax1, object_id, ref_point_x, ref_point_y, vx, vy, center_x, center_y, length, width, orientation, color=color, alpha=transparency)
    plt.grid()
    plt.show()

    fig2, ax2 = plt.subplots()
    plot_bb(ax2, center_x, center_y, length, width, orientation, color=color)
    plot_bb_orientation(ax2, center_x, center_y, length, width, orientation, color=color)
    plot_bb_ref_point(ax2, ref_point_x, ref_point_y, object_id, color=color)
    plot_bb_ref_point_vel_cart(ax2, ref_point_x, ref_point_y, vx, vy, color=color)
    plt.grid()
    plt.show()
