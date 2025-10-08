import datetime
import logging
import os
import traceback
from typing import Dict, List, Tuple

import numpy as np
import pandas as pd
from tqdm import tqdm

from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEObjectsEvaluationOutput import (
    PEObjectsEvaluationOutputMultiLog,
)
from aspe.evaluation.RadarObjectsEvaluation.Flags import IFlag
from aspe.evaluation.RadarObjectsEvaluation.KPI import (
    DEFAULT_BIN_CLASS_KPI_LIST,
    DEFAULT_PAIRS_KPI_DICT,
    BinaryClassificationKPIManager,
    IBinaryClassificationKPI,
    ISeriesKPI,
    PairsFeaturesKPIManager,
)
from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures.default_pairs_lifetime_kpis import (
    DEF_ACC_REL_LIFETIME_FEATURES_KPIS,
)
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline


class PEMultiLogEvaluation:
    """
    Class which runs PEPipeline over collection of logs. Results are aggregated to one dictionary which contains
    separate results for reference objects, estimated_objects and evaluated_logs_data.
    """

    def __init__(self, single_log_pipeline: PEPipeline, data_provider,
                 logging_file_folder: str,
                 pairs_features_kpis: Dict[str, List[Tuple[ISeriesKPI, IFlag]]] = DEFAULT_PAIRS_KPI_DICT,
                 objects_features_kpis: List[IBinaryClassificationKPI] = DEFAULT_BIN_CLASS_KPI_LIST,
                 pairs_lifetime_kpis: Dict[str, Tuple[ISeriesKPI]] = DEF_ACC_REL_LIFETIME_FEATURES_KPIS):
        """
        Initializes MultiLogPipeline with supplied PEPipeline and DataProvider.

        :param single_log_pipeline: PEPipeline object that is used for further evaluation of single log.
        :param data_provider:
        :param logging_file_folder: path to an existing directory where log files should be created
        """
        self.pe_output_multi_log = PEObjectsEvaluationOutputMultiLog()
        self._single_log_pipeline = single_log_pipeline
        self._data_provider = data_provider
        self._pairs_kpi_manager = PairsFeaturesKPIManager(pairs_features_kpis)
        self._objects_kpi_manager = BinaryClassificationKPIManager(objects_features_kpis)
        self._pairs_lifetime_kpis = pairs_lifetime_kpis
        self._logger = self._make_logger(logging_file_folder)

    @staticmethod
    def _make_logger(logging_file_folder: str) -> logging.Logger:
        """
        Creates and returns an instance of logger that is later used to
        record data processing steps or encountered errors

        :param logging_file_folder: path to an existing directory where individual log files should be created
        :return: logger: logger that has log file name, log record format and logging level configured
        """
        logger = logging.getLogger()

        logger.setLevel(logging.INFO)

        now = datetime.datetime.now().strftime('%Y%m%d%H%M%S')
        logging_file_name = f'pe_run_exceptions_log_{now}.log'
        logging_file_path = os.path.join(logging_file_folder, logging_file_name)

        handler = logging.FileHandler(logging_file_path, 'a')
        formatter = logging.Formatter('\n%(asctime)s-%(levelname)s-%(message)s')
        handler.setFormatter(formatter)
        logger.addHandler(handler)

        return logger

    def process_data(self, log_list: List[str], f_disable_exc_catch=False) -> PEObjectsEvaluationOutputMultiLog:
        """
        Runs evaluation on given log list and returns performance evaluation results aggregated to single dict.
        :param log_list: list of logs
        :param f_disable_exc_catch: True, if exceptions should be caught and ignored, or
                                    False, if MultiLogPipeline should crash when any data processing exception is thrown
        :return: aggregated data: dict with DataFrames containing evaluation results
        """
        for log_path in tqdm(log_list):
            try:
                message = f'\nStarted processing log {log_path}'
                self._logger.info(message)
                print(message)

                estimated_data, reference_data = self._data_provider.get_single_log_data(log_path)
                pe_output = self._single_log_pipeline.evaluate(estimated_data, reference_data)
                self.pe_output_multi_log.update(log_path, pe_output)

            except Exception as e:
                traceback.print_exc()
                self._logger.exception(f'\nError while processing log. ')
                if f_disable_exc_catch:
                    raise
                continue
        try:
            self._calculate_aggregated_results()
            self.pe_output_multi_log.print_kpis_and_log_features()
        except Exception as e:
            traceback.print_exc()
            self._logger.exception(f'\nError while processing log. ')
            if f_disable_exc_catch:
                raise

        return self.pe_output_multi_log

    def _calculate_aggregated_results(self):
        if len(self.pe_output_multi_log.pe_results_obj_pairs.signals) > 0:
            pairs_kpis = self._pairs_kpi_manager.calculate_kpis(self.pe_output_multi_log.pe_results_obj_pairs)
            objects_kpis = self._objects_kpi_manager.calculate_kpis(self.pe_output_multi_log.pe_results_obj_ref,
                                                                    self.pe_output_multi_log.pe_results_obj_est)
            lifetime_kpis = self.calculate_lifetime_kpis(self.pe_output_multi_log.pe_results_pairs_lifetime_features)
            log_features = self.pe_output_multi_log.logs_features_per_log.drop(columns='log_index').groupby(
                by='signature').sum().reset_index()

            self.pe_output_multi_log.kpis_pairs_features_aggregated = pairs_kpis
            self.pe_output_multi_log.kpis_binary_class_aggregated = objects_kpis
            self.pe_output_multi_log.kpis_pairs_lifetime_aggregated = lifetime_kpis
            self.pe_output_multi_log.logs_features_aggregated = log_features
        else:
            print('There is no associated pairs in evaluated log set. Skipping results aggregation')

    def calculate_lifetime_kpis(self, pairs_lifetime_features_results):
        results = {'feature_signature': [],
                   'kpi_signature': [],
                   'n_samples': [],
                   'kpi_value': [], }
        for feature_signature, kpis in self._pairs_lifetime_kpis.items():
            try:
                feature = pairs_lifetime_features_results.loc[:, feature_signature].to_numpy()
                feature = feature[~np.isnan(feature)]
                for kpi in kpis:
                    if len(feature) > 0:
                        kpi_value = kpi.calculate_kpi(feature)
                    else:
                        kpi_value = np.nan
                    results['feature_signature'].append(feature_signature)
                    results['kpi_signature'].append(kpi.kpi_signature)
                    results['n_samples'].append(len(feature))
                    results['kpi_value'].append(kpi_value)
            except KeyError:
                print(f'calculate_lifetime_kpis: Missing feature: {feature_signature} ')
        return pd.DataFrame(results)
