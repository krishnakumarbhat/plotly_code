import numpy as np
import pandas as pd


def get_samples(data_set, method=None, size=None, clusters=None, n_clusters=None):
    sample = Sample(data_set=data_set, size=size)
    if method == 'stratified':
        return sample.get_stratified_sample(clusters=clusters, n_clusters=n_clusters)
    elif method == 'random':
        return sample.get_random_sample()
    elif method == 'equal_count':
        return sample.get_equal_count_sample(clusters=clusters, n_clusters=n_clusters)


class Sample:
    def __init__(self, data_set, size):
        self.data_set = data_set
        self.size = size

    def get_random_sample(self):
        i = np.random.choice(self.data_set.index.to_numpy(), int(self.size))
        return self.data_set.iloc[i, :]

    def get_stratified_sample(self, clusters=None, n_clusters=None):
        data = pd.DataFrame()
        cluster_count = clusters.groupby('cluster')['cluster'].count().to_dict()
        for c in range(n_clusters):
            n_logs_from_cluster = cluster_count[c] / self.data_set.count()[0] * self.size
            if n_logs_from_cluster < 1:
                num = np.ceil(n_logs_from_cluster)
            else:
                num = np.rint(n_logs_from_cluster)
            i = np.random.choice(clusters[clusters['cluster'] == c].index.to_numpy(), int(num))
            data = data.append(self.data_set.iloc[i, :], ignore_index=True, sort=False)

        if self.size != data.shape[0]:
            data = self._equalise_size(data)

        return data

    def get_equal_count_sample(self, clusters, n_clusters):
        data = pd.DataFrame()
        count = int(np.ceil(self.size / n_clusters))
        cluster_count = clusters.groupby('cluster')['cluster'].count().to_dict()
        for c in range(n_clusters):
            count = min(count, cluster_count[c])
            i = np.random.choice(clusters[clusters['cluster'] == c].index.to_numpy(), int(count))
            data = data.append(self.data_set.iloc[i, :], ignore_index=True, sort=False)

        if self.size != data.shape[0]:
            data = self._equalise_size(data)

        return data

    def _equalise_size(self, data):
        diff = data.shape[0] - self.size
        if diff > 0:
            i = np.random.choice(data.index.to_numpy(), int(abs(diff)))
            data = data.drop(index=i, axis=0)
        elif diff < 0:
            x = self.data_set.path.to_numpy()
            y = data.path.to_numpy()
            idxs = np.where(y.reshape(y.size, 1) == x)[1]
            df = self.data_set.drop(index=idxs, axis=0)
            df.reset_index(inplace=True)
            df = df.drop('index', axis=1)
            additional_logs = df.iloc[np.random.choice(df.index.to_list(), int(abs(diff))), :]
            data = data.append(additional_logs, ignore_index=True, sort=False)
        return data
