import pickle
import numpy as np
from scipy.stats import gaussian_kde


def calc_quantils(data):
    abs_data = np.abs(data)
    quant_50 = np.quantile(abs_data, 0.5)
    quant_95 = np.quantile(abs_data, 0.95)
    quant_99 = np.quantile(abs_data, 0.99)
    return quant_50, quant_95, quant_99


def load_pickle(pickle_path):
    with open(pickle_path, 'rb') as handle:
        data = pickle.load(handle)
    return data


def calc_cdf(x_abs, y_data):
    """Calculate CDF"""
    if len(y_data) != 0:
        y_data = y_data.astype('float64')
        y_data = y_data[~np.isnan(y_data)]
        xp = np.sort(np.abs(y_data))
        yp = np.array(range(len(xp))) / float(len(xp))
        if len(xp) != 0  and len(yp) !=0 :
        	y_cdf = np.interp(x_abs, xp, yp)
        else:
            y_cdf = np.nan
    else:
        y_cdf = np.nan
    return y_cdf


def calc_pdf(x_data, y_data):
    if len(y_data) != 0:
        y_data = y_data.astype('float64')
        y_data = y_data[~np.isnan(y_data)]  # delete nan from vector

        left = x_data.min()
        right = x_data.max()
        y_data = y_data[(y_data >= left) & (y_data <= right)]
        if len(y_data) != 0 and len(x_data) != 0:
        	kernel = gaussian_kde(y_data)
        	y_pdf = kernel(x_data)
        else:
            y_pdf = np.nan
    else:
        y_pdf = np.nan
    return y_pdf
