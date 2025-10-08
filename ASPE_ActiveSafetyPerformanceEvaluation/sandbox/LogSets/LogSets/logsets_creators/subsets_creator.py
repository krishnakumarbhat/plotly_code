from LogSets.utils.file_handling import load
from LogSets.logsets_creators.subsets_creators.outlier_set_creator import OutlierSetCreator
from LogSets.logsets_creators.subsets_creators.representative_set_creator import RepresentativeSetCreator


class SubSetsCreator:
    def __init__(self, full_set, alpha_set_path, golden_set_path, outlier_set_path):
        """
        :param full_set: data frame, full set of logs with calculated components
        :param alpha_set_path: save path for alpha set
        :param golden_set_path: save path for golden set
        :param outlier_set_path: save path for outliers set
        """
        self.full_set = full_set
        self.creators = dict()
        self.alpha_n_logs = 25
        self.golden_n_logs = 100
        self._init_sub_sets_creators(alpha_set_path, golden_set_path, outlier_set_path)

    def _init_sub_sets_creators(self, alpha_set_path, golden_set_path, outlier_set_path):
        self.creators['alpha'] = RepresentativeSetCreator(alpha_set_path, self.full_set)
        self.creators['golden'] = RepresentativeSetCreator(golden_set_path, self.full_set)
        self.creators['outlier'] = OutlierSetCreator(outlier_set_path, self.full_set)
        
    def create_all(self, plot, plot_clusters):
        """
        Create all subsets
        :param plot: flag for plotting 3D plot of dataset
        :param plot_clusters: flag for plotting clusters on 3d plot
        :return:
        """
        self.creators['alpha'].create(n_logs=self.alpha_n_logs, plot=plot, plot_clusters=plot_clusters)
        self.creators['golden'].create(n_logs=self.golden_n_logs, plot=plot, plot_clusters=plot_clusters)
        self.creators['outlier'].create(plot=plot)
        
    def save_all(self):
        for creator in self.creators.values():
            creator.save()


def example():
    alpha_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\AlphaSets\alpha_set.pickle'
    golden_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\GoldenSets\golden_set.pickle'
    outlier_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\OutlierSets\outlier_set.pickle'
    full_set = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\fullset_test.pickle')
    sub_sets_creator = SubSetsCreator(full_set, alpha_set_path, golden_set_path, outlier_set_path)
    sub_sets_creator.create_all(plot=True, plot_clusters=True)
    sub_sets_creator.save_all()


if __name__ == '__main__':
    example()
