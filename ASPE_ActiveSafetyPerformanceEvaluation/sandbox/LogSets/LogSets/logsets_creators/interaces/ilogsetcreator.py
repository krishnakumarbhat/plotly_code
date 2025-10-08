from abc import ABC, abstractmethod
import json
import pandas as pd
from sklearn.preprocessing import StandardScaler
from LogSets.utils.file_handling import load, save


class ILogSetCreator(ABC):
    def __init__(self):
        self.components = pd.DataFrame()
        self.path = str()
        self.n_logs = int()
        self.resim_version = 'rRf360t4010304v202r1'
        self.hyper_params = dict()
        self._load_hyper_params()

    @abstractmethod
    def create(self):
        return self.components

    def save(self, path=None, add_txt_file=True):
        """
        :param path: optional, save path
        :param add_txt_file: optional, if True then creates text file containing logs paths
        """
        if path is not None:
            self.path = path
        save(self.components, self.path)

        if add_txt_file:
            with open(self.path.replace('.pickle', '.txt'), 'w') as handle:
                for line in self.components['path']:
                    handle.write(line + '\n')

    def load(self):
        """
        Loading data sets to self.components attribute
        """
        self.components = load(self.path)
        return self.components

    @staticmethod
    def _get_normalized(full_set):
        """
        Drop path columns (categorical) and normalize data set
        :param full_set: data frame with logs
        :return:
        """
        full_set_num = full_set.drop('path', axis=1)
        return StandardScaler().fit_transform(full_set_num)

    def _load_hyper_params(self):
        """
        Load hyper parameters for subset's AUTO methods
        """
        with open(r"\\10.224.186.68\AD-Shared\F360\Tools\LogSets\HyperParameters\hyper_params.json", 'r') as file:
            self.hyper_params = json.load(file)

    def save_hyper_params(self, alpha_set_params, golden_set_params, outlier_set_params):
        """
        Save hyper parameters for subset's AUTO methods
        :param alpha_set_params: dictionary with alpha set hyper params
        :param outlier_set_params: dictionary with outlier set hyper params
        :param golden_set_params: dictionary with golden set hyper params
        """
        self.hyper_params = {'alpha_set': alpha_set_params,
                  'golden_set': golden_set_params,
                  'outlier_set': outlier_set_params}
        with open(r"\\10.224.186.68\AD-Shared\F360\Tools\LogSets\HyperParameters\hyper_params.json", 'w') as file:
            json.dump(self.hyper_params, file, indent=4)
