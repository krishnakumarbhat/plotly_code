from LogSets.logsets_creators.interaces.ilogsetcreator import ILogSetCreator
from LogSets.utils.clustering.cluster import get_clusters
from LogSets.utils.file_handling import load
from LogSets.utils.pca.pca import get_princ_comps
from LogSets.utils.sampling.sample import get_samples
from LogSets.utils.plots.plots import plot3d_dataset


class RepresentativeSetCreator(ILogSetCreator):
    """
    AlphaSet and GoldenSet creator
    """
    def __init__(self, path, full_set):
        """
        :param path: set's path
        :param full_set: data frame, full set of logs with calculated components
        """
        super().__init__()
        self.path = path
        self.full_set = full_set

    def create(self, n_logs=None, method='auto', n_components=None, n_clusters=None, cluster_method=None, quantile=None,
               pca_method=None, plot=None, plot_clusters=None, append_n_logs=True, sample_method=None,
               **kwargs):
        """
        :param n_logs: size of new data set ( if n_logs < 50 data set's name is alpha otherwise golden)
        :param method: optional, creation method, available methods = ('auto', 'pca_clustering')
        :param n_components: optional, number of principal components to create using PCA
        :param n_clusters: optional, number of clusters to create in K-means clustering
        :param cluster_method: optional, clustering method, available methods = ('k-means', 'mean shift')
        :param quantile: optional, param's value for mean shift clustering
        :param pca_method: optional, PCA method, available methods = ('pca', 'kernel_pca')
        :param plot: optional, flag for plotting 3D plot of dataset
        :param plot_clusters: optional, flag for plotting clusters on 3d plot
        :param append_n_logs: optional, appending number of logs to dataset name
        :param sample_method: optional, sampling for pca_clustering, methods = ('random', 'stratified', 'equal_count')
        :param kwargs:
        :return: created data set, 2D df
        """
        if n_logs is not None:
            if n_logs > self.full_set.shape[0]:
                raise ValueError('Number of logs in subset = %d cant be larger than number of logs = %d in full set'
                                 % (n_logs, self.full_set.shape[0]))
            else:
                self.n_logs = n_logs
        else:
            raise ValueError('Please provide number of logs (n_logs=)')

        self.full_set_norm = self._get_normalized(self.full_set)

        if method == 'auto':
            if n_logs <= 25:
                params = self.hyper_params['alpha_set']
            else:
                params = self.hyper_params['golden_set']
            self.components, self.clusters = self._pca_clustering(full_set_norm=self.full_set_norm,
                                                                  n_components=params['n_components'],
                                                                  pca_method=params['pca_method'],
                                                                  cluster_method=params['cluster_method'],
                                                                  n_clusters=params['n_clusters'],
                                                                  quantile=params['quantile'],
                                                                  sample_method=params['sample_method'])

        elif method == 'pca_clustering':
            self.components, self.clusters = self._pca_clustering(full_set_norm=self.full_set_norm,
                                                                  n_components=n_components,
                                                                  pca_method=pca_method, cluster_method=cluster_method,
                                                                  n_clusters=n_clusters, quantile=quantile,
                                                                  sample_method=sample_method)
        else:
            raise NotImplementedError

        self.n_logs = self.components.shape[0]

        if self.n_logs > 99:
            title = 'GoldenSet'
            self.path = self.path.replace('AlphaSets', 'GoldenSets')
        else:
            title = 'AlphaSet'

        if append_n_logs:
            self.path = self.path.replace('.pickle', '_' + str(self.n_logs) + '.pickle')

        if plot:
            if not plot_clusters:
                self.clusters = None
            plot3d_dataset(self.full_set, self.components, path=self.path, plot_title=title, clusters=self.clusters,
                           **kwargs)

        return self.components

    def _pca_clustering(self, full_set_norm=None, n_components=None, pca_method=None, cluster_method=None,
                        n_clusters=None, quantile=None, sample_method=None):
        """
        :param full_set_norm: normalized full data set
        :param n_clusters: optional, number of clusters to create in K-means clustering
        :param cluster_method: optional, clustering method, available methods = ('k-means', 'mean shift')
        :param quantile: optional, param's value for mean shift clustering
        :param pca_method: optional, PCA method, available methods = ('pca', 'kernel_pca')
        :param sample_method: optional, sampling for pca_clustering, methods = ('random', 'stratified', 'equal_count')
        :return: sampled set, 2D df
        """
        full_set_pca = get_princ_comps(full_set_norm, n_components=n_components, method=pca_method)
        clusters, n_clusters = get_clusters(full_set_pca, method=cluster_method, n_clusters=n_clusters,
                                            quantile=quantile)
        return get_samples(self.full_set, method=sample_method, size=self.n_logs, clusters=clusters,
                           n_clusters=n_clusters), clusters


def example():
    full_set = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\fullset_dropped_comps.pickle')
    representative_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\AlphaSets\sample_set.pickle'
    representative_set = RepresentativeSetCreator(representative_set_path, full_set)
    representative_set.create(n_logs=25, plot=True, plot_clusters=True)
    representative_set.save()
    print()


def demo():
    full_set = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\fullset_dropped_comps.pickle')
    representative_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\AlphaSets\sample_set.pickle'
    representative_set = RepresentativeSetCreator(representative_set_path, full_set)
    comps = representative_set.create(n_logs=25, plot=False, plot_clusters=False)
    plot3d_dataset(full_set, comps, path=representative_set.path, plot_title='AlphaSet', clusters=None)
    plot3d_dataset(full_set, comps, path=representative_set.path, plot_title='AlphaSet',
                   clusters=representative_set.clusters)
    print()


if __name__ == '__main__':
    demo()
    example()
