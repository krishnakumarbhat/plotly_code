from LogSets.logsets_creators.interaces.ilogsetcreator import ILogSetCreator
from LogSets.utils.file_handling import load
from LogSets.utils.util_funcs import get_p_value_from_ww_test, get_avg_clusters_quality
from LogSets.utils.plots.plots import plot3d_dataset
from tqdm import tqdm
import numpy as np
from LogSets.logsets_creators.subsets_creators.representative_set_creator import RepresentativeSetCreator
from LogSets.logsets_creators.subsets_creators.outlier_set_creator import OutlierSetCreator


class FullSetCreator(ILogSetCreator):
    def __init__(self, path):
        """
        :param path: path to full set
        """
        super().__init__()
        self.path = path

    def create(self, components=None, fit=None, plot=None, drop_features_full_of_zeros=None,
               drop_features_with_nan_values=None, **kwargs):
        """
        :param components: data frame with calculated features and/or events
        :param fit: optional, flag for finding best parameters for creation of subsets
        :param plot: optional, flag for plotting set in as 3d plot
        :param drop_features_full_of_zeros: optional, flag to drop columns that have only zeros
        :param drop_features_with_nan_values: optional, flag to drop columns that have only nans
        :param kwargs:
        :return: created full_set
        """
        if components is None:
            raise AttributeError
        self.components = components.copy(deep=True)
        if drop_features_with_nan_values:
            self.components = self.components.dropna(axis=1, how='all')
        if drop_features_full_of_zeros:
            self.components = self.components.loc[:, (self.components != 0).any(axis=0)]
        self.components = self.components.fillna(0)

        if fit:
            self.fit()

        if plot:
            plot3d_dataset(self.components, path=self.path, plot_title='Full set', **kwargs)

        return self.components

    def fit(self):
        alpha_set_hyper_params = self._representative_set_optimizer(n_logs=25)
        golden_set_hyper_params = self._representative_set_optimizer(n_logs=100)
        outlier_set_hyper_params = self._outlier_set_optimizer(size_percentage_range=(0.02, 0.05))
        self.save_hyper_params(alpha_set_hyper_params, golden_set_hyper_params, outlier_set_hyper_params)

    def _representative_set_optimizer(self, n_logs):
        """
        Optimizer of hyper parameters for creation of representative subsets.
        Optimizer maximizes profit function which is p-value (0-1 value meaning how much two distributions are
        similar to each other) calculated from kolmogorov_smirnov good of fitness test. ( Subset's good of fitness to
        full set). In simple terms: We create sample representative sets in loop with chosen parameters and if
        p_value is larger than best p_value found before than we assign it to best p_value and continue...
        :param n_logs: number of logs in subset that parameters are optimized for
        :return: best parameters
        """
        p_best = 0
        params = {}
        full_set = self.components.drop('path', axis=1)

        #   Optimizers parameters
        params['method'] = 'pca_clustering'
        params['sample_method'] = 'stratified'
        c_method = 'kmeans'
        pca = 'kernel_pca'
        min_n_components = 3
        max_n_components = 15
        min_clusters = 3
        max_clusters = 21

        for n_comps in tqdm(range(min_n_components, max_n_components, 2),
                            desc='Tuning parameters for Sample set of ' + str(n_logs)):
            for n_clusters in range(min_clusters, max_clusters, 2):
                data_set = RepresentativeSetCreator('', self.components)
                data_set.create(n_logs=n_logs, method=params['method'], n_components=n_comps,
                                n_clusters=n_clusters, cluster_method=c_method, pca_method=pca,
                                sample_method=params['sample_method'], plot=False)

                data_set.components = data_set.components.drop('path', axis=1)
                p_value = get_p_value_from_ww_test(full_set, data_set.components)
                if p_value > p_best:
                    p_best = p_value
                    params['n_components'], params['pca_method'], params['cluster_method'], params['n_clusters'], \
                    params['p_value'], params['n_logs'], params['quantile'] = n_comps, pca, c_method, n_clusters, \
                                           round(p_best, 2), n_logs, None

        print(f'Similarity between distributions: {int(params["p_value"] * 100)}% \n Best parameters: {params}')
        return params

    def _outlier_set_optimizer(self, size_percentage_range=(0.03, 0.05)):
        """
        Optimizer of hyper parameters for creation of outliers subset.
        Optimizer maximizes profit function which cluster quality calculated with. In simple terms: We create sample
        outliers sets in loop with chosen parameters and if value of profit_function is larger than best cluster
        quality found before than we assign its value to best cluster quality and continue...
        :param size_percentage_range: low and high limit of percentage of logs in full set that might be outliers
        :return: best parameters
        """
        #   Optimizers parameters
        best_cluster_quality = -1
        params = {}
        n_components = 20
        min_eps = 1
        max_eps = 15
        min_of_min_samples = 1
        max_of_min_samples = 10
        cluster_method = 'dbscan'
        pca = True
        low = size_percentage_range[0]
        high = size_percentage_range[1]

        for eps in tqdm(range(min_eps, max_eps), desc='Tuning parameters for Outlier set'):
            for min_samples in range(min_of_min_samples, max_of_min_samples):
                outlier = OutlierSetCreator('', self.components)
                full_num = outlier._get_normalized(outlier.full_set)
                subset, cluster_labels = outlier._get_outliers_by_clustering(full_set_num=full_num,
                                                                             method="dbscan", eps=eps,
                                                                             min_samples=min_samples, pca=pca,
                                                                             n_components=n_components)

                if subset.size == 0 or np.unique(cluster_labels).shape[0] < 2:  # no outliers found
                    continue
                profit_function = get_avg_clusters_quality(full_num, cluster_labels)
                if best_cluster_quality < profit_function and int(self.components.shape[0] * low) < subset.shape[0]<int(
                        self.components.shape[0] * high):
                    best_cluster_quality = profit_function
                    params = {'method': 'clustering',
                              'eps': eps,
                              'min_samples': min_samples,
                              'cluster_quality': round(best_cluster_quality, 2),
                              'pca': pca,
                              'n_components': n_components,
                              'cluster_method': cluster_method,
                              'n_logs': subset.shape[0]}

        print(f'Outlier"s set clusters quality: {round(params["cluster_quality"], 2)} \n Best parameters: {params}')
        return params


def example():
    components = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\raw_components\components.pickle')
    creator = FullSetCreator(path=r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\fullset_dropped_comps.pickle')
    fullset = creator.create(components, plot=True, fit=True, drop_features_with_nan_values=True,
                             drop_features_full_of_zeros=True)
    creator.save()
    print()


def example_with_parsing_extracting_features():
    from LogSets.main import LogSets
    #   STEP 3: Find logs paths that were re-simulated with provided resim version
    re_simulated_logs_paths = LogSets.find_re_simulated_logs_by_resim_version(resim_ver='rRf360t4010304v202r1')

    #   STEP 4: Parse, extract and the calculate features and events for re-simulated logs in loop
    components = LogSets.calculate_all_components(re_simulated_logs_paths)

    #   STEP 5: Create FUll Set
    creator = FullSetCreator(path=r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\fullset_test.pickle')
    fullset = creator.create(components, plot=True, fit=True)
    creator.save()
    print()


if __name__ == '__main__':
    example()
    # example_with_parsing_extracting_features()


