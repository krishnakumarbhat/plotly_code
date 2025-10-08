"""coding=utf-8"""
from dataclasses import dataclass, field
from typing import List
import numpy as np


@dataclass
class Diagnostic:
    """
    general diagnostic class
    """
    f_all_signals_extracted: bool = field(default_factory=bool)
    f_default_checked: bool = field(default_factory=bool)
    f_raw_signals_checked: bool = field(default_factory=bool)

    unextracted_signals_list: List[str] = field(default_factory=list)
    unextracted_raw_signals_list: List[str] = field(default_factory=list)
    dataset_scan_indexes_inconsistency: List[str] = field(default_factory=list)

    @staticmethod
    def default_build_check(force: bool = False):
        """
        decorator for build function from IBuilder, allows check if all there is no nan signal
        :param force: if we want to force checking signals each time we call, if set to false when ones
        """
        def decorator(func):
            """
            simple check is theres any colums filled with nan, if theres any it:
            :unextracted_signals_list list: calculate list with nan signals
            :f_all_signals_extracted bool: set flag if theres all signals extracted
            :param func: function which is decorated
            :return:
            """

            def wrapper(self):
                """
                wrapper that calls decorated function
                :param self: argument passing to decorated function, this shall be IBuilder's self
                :return: 
                """
                from AptivDataExtractors.interfaces.IDataSet import IDataSet
                dataset: IDataSet = func(self)
                diag = dataset.diagnostic  # shortcut to be more convienient 

                if force or diag.f_default_checked is None:
                    diag.unextracted_signals_list = dataset.signals.columns[dataset.signals.isnull().any()].tolist()

                    if diag.unextracted_signals_list:
                        diag.f_all_signals_extracted = False
                    else:
                        diag.f_all_signals_extracted = True

                    diag.f_default_checked = True
                return dataset

            return wrapper

        return decorator

    @staticmethod
    def raw_signals_check(func):
        """
        decorator that checking if all raw signals from parsers has been extracted
        :param func: func (decorated function) shall return list of names of all possible extrated signals
        :return:
        """

        def wrapper(self):
            """
            wrapper comparing all raw signals and extracted signals and difference in stored in
            unextracted_raw_signals_list
            :param self: shall be IBuilder's self
            :return:
            """
            dataset = self.dataset

            raw_signals = func(self)
            extracted_signals = dataset.signals.columns
            dataset.diagnostic.unextracted_raw_signals_list = list(set(raw_signals) - set(extracted_signals))

            dataset.diagnostic.f_raw_signals_checked = True
            return raw_signals

        return wrapper

    @staticmethod
    def all_datasets_scanindex_consistency(func):
        """
        decorator for extract_data function, checking all which scan indexes are not matching
        expecting that extract_data will return ExtractedData
        :param func: extract_data
        :return:
        """

        def wrapper(*args):
            """
            storing all scan idxes arrays in scan_idxes_dict for each dataset.
            creating mask with rows=dataset number, columns=max length of scan indexes.
            every inconsistency in scanindexes for each dataset is saved to diagnostic data in particualr dataset
            :param args:
            :return:
            """
            from AptivDataExtractors.interfaces.ExtractedData import ExtractedData
            extracted_data: ExtractedData = func(*args)
            extracted_data_dict = extracted_data.data.to_dict()  # seting dict to var for more convinient handling

            scan_idxes_dict: {'dataset_name': np.ndarray} = {}

            min_overall_scan_idx = 9999999  # not so happy of this solution
            max_overall_scan_idx = -1

            items_num = 0
            for dataset_name, dataset in extracted_data_dict.items():
                items_num += 1

                scan_idxes_dict[dataset_name] = dataset.signals['scan_idx'].unique()  # storing each scan idx array

                # on these min and max value lenght of mask will be based
                if scan_idxes_dict[dataset_name].min() < min_overall_scan_idx:
                    min_overall_scan_idx = scan_idxes_dict[dataset_name].min()
                if scan_idxes_dict[dataset_name].max() > max_overall_scan_idx:
                    max_overall_scan_idx = scan_idxes_dict[dataset_name].max()

            # initial mask with all scan indexes
            scan_idx_mask = np.full((items_num, int(max_overall_scan_idx - min_overall_scan_idx + 1)), 0)

            for idx, (name, array) in enumerate(scan_idxes_dict.items()):
                scan_idx_mask[idx][array] = 1  # where scan index is existing, setting value to 1
                missing_scan_idxes = np.argwhere(scan_idx_mask[idx] == 0).tolist()
                extracted_data_dict[name].diagnostic.dataset_scan_indexes_inconsistency = missing_scan_idxes

            # currently commented, this mask can be stored in main diagnostic structure
            # but leaving this comnented so far
            # missing_scan_idxes_array = np.argwhere(scan_idx_mask == 0)

            return extracted_data

        return wrapper
