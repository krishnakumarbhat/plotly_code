from LogSets.logsets_creators.interaces.ilogsetcreator import ILogSetCreator
from LogSets.utils.file_handling import load
from LogSets.utils.plots.plots import plot3d_dataset
from LogSets.utils.pca.pca import get_princ_comps
from LogSets.utils.clustering.cluster import Cluster


class OutlierSetCreator(ILogSetCreator):
    def __init__(self, path, full_set):
        """
        :param path: set's path
        :param full_set: data frame, full set of logs with calculated components
        """
        super().__init__()
        self.path = path
        self.full_set = full_set

    def create(self, method='auto', plot=True, append_n_logs=True, eps=None, min_samples=None, pca=None,
               n_components=None, **kwargs):
        """
        :param method: optional, creation method, available methods = ('auto', 'clustering')
        :param plot: optional, flag for plotting 3D plot of data set
        :param append_n_logs: optional, appending number of logs to data set's path
        :param eps: optional, distance between points
        :param min_samples: optional, min number of samples in order to create cluster
        :param pca: optional, flag, dimension reduction using Principal Components Analysis
        :param n_components: optional, number of principal components to create using PCA
        :param kwargs:
        :return: created data set, 2D df
        """
        full_set_norm = self._get_normalized(self.full_set)

        if method == 'auto':
            params = self.hyper_params['outlier_set']
            self.components, _ = self._get_outliers_by_clustering(full_set_num=full_set_norm, method=params['cluster_method'],
                                                                  pca=params['pca'], eps=params['eps'],
                                                                  min_samples=params['min_samples'], n_components=params['n_components'])
        elif method == 'clustering':
            self.components, _ = self._get_outliers_by_clustering(full_set_num=full_set_norm, method="dbscan", eps=eps,
                                                                  min_samples=min_samples, pca=pca, n_components=n_components)
        else:
            raise NotImplementedError

        self.n_logs = self.components.shape[0]

        if append_n_logs:
            self.path = self.path.replace('.pickle', '_' + str(self.n_logs) + '.pickle')

        if plot:
            plot3d_dataset(self.full_set, self.components, path=self.path, plot_title='Outliers set', **kwargs)

        return self.components

    def _get_outliers_by_clustering(self, full_set_num=None, method=None, min_samples=None, pca=None, eps=None, n_components=None):
        """
        Return outliers using dbscan clustering
        :param full_set_num: numpy array
        :param method: clustering method
        :param min_samples: min number of samples in order to create cluster
        :param pca: flag, dimension reduction using Principal Components Analysis
        :param eps: epsilon, distance between points
        :return: outliers, 2D df
        """
        if pca:
            full_set_num = get_princ_comps(full_set_num, n_components=n_components, method='kernel_pca')
        if method == 'dbscan':
            cluster = Cluster(data_set=full_set_num)
            idxs, cluster_labels = cluster.get_outliers_using_dbscan(eps=eps, min_samples=min_samples)
            return self.full_set.iloc[idxs[0], :], cluster_labels
        else:
            raise NotImplementedError


def example():
    full_set = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\fullset_dropped_comps.pickle')
    outlier_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\OutlierSets\outlier_set.pickle'
    outlier_set = OutlierSetCreator(outlier_path, full_set)
    outlier_set.create()
    outlier_set.save()
    print()


if __name__ == '__main__':
    example()
