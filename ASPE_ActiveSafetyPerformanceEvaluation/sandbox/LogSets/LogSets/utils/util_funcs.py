import numpy as np
import pandas as pd
import scipy.stats as stats
from sklearn.neighbors import kneighbors_graph
from scipy.sparse.csgraph import minimum_spanning_tree
from LogSets.utils.pca.pca import get_princ_comps
from sklearn.metrics import silhouette_samples, silhouette_score


def get_kolmogorov_smirnov_test_results(data_set_1, data_set_2):
    """
    Kolmogorov-Smirnov Goodness of Fit Test
    :param data_set_1: 1D or 2D numpy array or data frame
    :param data_set_2: 1D or 2D numpy array or data frame
    :return: distance, p_value
    """
    try:
        data_set_1 = data_set_1.to_numpy()
        data_set_2 = data_set_2.to_numpy()
    except:
        pass
    ds1_n_features = data_set_1.shape[1]
    ds2_n_features = data_set_2.shape[1]

    if ds1_n_features != ds2_n_features:
        print('Data sets needs to have same number of features')
        exit()

    def ks_test_1d(arr1, n=data_set_2.shape[0]):
        return stats.ks_2samp(arr1[0:-n], arr1[-n:])

    if ds1_n_features and ds2_n_features == 1:
        return stats.ks_2samp(data_set_1, data_set_2)
    else:
        d, p = np.apply_along_axis(ks_test_1d, 0, np.append(data_set_1, data_set_2, axis=0))
        return d.mean(), p.mean()


def calc_z_value(dataset, threshold=2, pca=None):
    data_num = dataset.drop('path', axis=1)
    if pca:
        data_num = get_princ_comps(data_num, n_components=3, method='kernel_pca')
    z = np.abs(stats.zscore(data_num))
    return dataset[(z > threshold).all(axis=1)]


def mst_edges(V, k):
    """
    Construct the approximate minimum spanning tree from vectors V
    :param: V: 2D array, sequence of vectors
    :param: k: int the number of neighbor to consider for each vector
    :return: V ndarray of edges forming the MST
    """

    # k = len(X)-1 gives the exact MST
    k = min(len(V) - 1, k)

    # generate a sparse graph using the k nearest neighbors of each point
    G = kneighbors_graph(V, n_neighbors=k, mode='distance')

    # Compute the minimum spanning tree of this graph
    full_tree = minimum_spanning_tree(G, overwrite=True)

    return np.array(full_tree.nonzero()).T


def ww_test(X, Y, k=20):
    """
    Multi-dimensional Wald-Wolfowitz test
    :param X: multivariate sample X as a numpy ndarray
    :param Y: multivariate sample Y as a numpy ndarray
    :param k: number of neighbors to consider for each vector
    :return: W the WW test statistic, R the number of runs
    """
    m, n = len(X), len(Y)
    N = m + n

    XY = np.concatenate([X, Y]).astype(np.float)

    # XY += np.random.normal(0, noise_scale, XY.shape)

    edges = mst_edges(XY, k)

    labels = np.array([0] * m + [1] * n)

    c = labels[edges]
    runs_edges = edges[c[:, 0] == c[:, 1]]

    # number of runs is the total number of observations minus edges within each run
    R = N - len(runs_edges)

    # expected value of R
    e_R = ((2.0 * m * n) / N) + 1

    # variance of R is _numer/_denom
    _numer = 2 * m * n * (2 * m * n - N)
    _denom = N ** 2 * (N - 1)

    # see Eq. 1 in Friedman 1979
    # W approaches a standard normal distribution
    W = (R - e_R) / np.sqrt(_numer/_denom)

    return W, R


def get_p_value_from_ww_test(X, Y):
    W, R = ww_test(X, Y)
    pvalue = stats.norm.cdf(W)
    return pvalue


def get_avg_clusters_quality(X, cluster_labels):
    # The best value is 1 and the worst value is -1. Values near 0 indicate overlapping clusters.
    # Negative values generally indicate that a sample has been assigned to the wrong cluster,
    # as a different cluster is more similar.

    # The silhouette_score gives the average value for all the samples.
    # This gives a perspective into the density and separation of the formed clusters
    silhouette_avg = silhouette_score(X, cluster_labels)
    return silhouette_avg

