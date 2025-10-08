import numpy as np


def interp_3d(x: np.array, xp: np.array, yp_3d: np.array):
    """

    :param x:
    :param xp:
    :param yp_3d:
    :return:
    """
    size_out = [x.shape[0], yp_3d.shape[1], yp_3d.shape[2]]
    yp_3d_out = np.zeros(size_out)

    for i in range(size_out[1]):
        for j in range(size_out[2]):
            yp_3d_out[:, i, j] = np.interp(x, xp, fp=yp_3d[:, i, j])

    return yp_3d_out
