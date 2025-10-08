import numpy as np
import pandas as pd
from sklearn.cluster import KMeans, MeanShift, estimate_bandwidth, DBSCAN


def get_clusters(data_set, method='kmeans', n_clusters=None, quantile=None):
    cluster = Cluster(data_set=data_set)
    if method == 'kmeans':
        return cluster.get_kmeans_clusters(n_clusters=n_clusters)
    elif method == 'mean_shift':
        return cluster.get_mean_shift_clusters(quantile=quantile)


class Cluster:
    def __init__(self, data_set):
        self.data_set = data_set

    def get_kmeans_clusters(self, n_clusters=None):
        kmeans = KMeans(n_clusters=n_clusters)
        kmeans = kmeans.fit(self.data_set)
        labels_clustering = kmeans.predict(self.data_set)
        return pd.DataFrame(labels_clustering, columns=['cluster']), n_clusters

    def get_mean_shift_clusters(self, quantile=None):
        bandwidth = estimate_bandwidth(self.data_set, quantile=quantile, n_samples=self.data_set.shape[0])
        mshift = MeanShift(bandwidth=bandwidth).fit(self.data_set)
        labels_clustering = mshift.predict(self.data_set)
        n_clusters = len(np.unique(mshift.labels_))
        return pd.DataFrame(labels_clustering, columns=['cluster']), n_clusters

    def get_outliers_using_dbscan(self, eps=None, min_samples=None):
        dbscan = DBSCAN(eps=eps, min_samples=min_samples)
        cluster_labels = dbscan.fit_predict(self.data_set)
        _, n = np.unique(cluster_labels, return_counts=True)
        return np.where(cluster_labels == -1), cluster_labels
