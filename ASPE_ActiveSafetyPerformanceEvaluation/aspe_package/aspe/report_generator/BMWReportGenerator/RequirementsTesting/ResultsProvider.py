import os
import re
from copy import deepcopy
import numpy as np
import pandas as pd
from srr5_dev_tools.common import NestedDict
from tqdm import tqdm

from aspe.utilities.SupportingFunctions import save_to_pkl
from aspe.providers import F360MdfRTRangeDataProvider
from aspe.providers.support_functions import get_log_list_from_path
from aspe.evaluation.RadarObjectsEvaluation.Association import RelEstToRefNN
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.PairsFeaturesKPIManager import EXTENDED_PAIRS_KPI_DICT
from aspe.evaluation.RadarObjectsEvaluation.PEMultiLogEvaluation import PEMultiLogEvaluation
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import EXTENDED_PAIRS_FEATURES_LIST
from aspe.evaluation.RadarObjectsEvaluation.PairsPreBuilders import PEPairedObjectsPreBuilderGating
from aspe.report_generator.BMWReportGenerator.RequirementsTesting.BMW_ASPE_SIGNAL_MAPPER import BMW_ASPE_SIGNAL_MAPPER
from aspe.report_generator.BMWReportGenerator.RequirementsTesting.KPI_FILTERS import KPI_FILTERS
from aspe.report_generator.BMWReportGenerator.RequirementsTesting.RELEVANCY_FILTERS import DS_FILTERS
from aspe.report_generator.BMWReportGenerator.RequirementsTesting.REQ.BMW_ASPE_KPI_MAPPER import BMW_ASPE_KPI_MAPPER, ASPE_LATENCY_MAPPING

"""
Run this script as 'Run File in Console'
Example of evaluating performance when several logs should be evaluated.
This class is configured by data provider and PE Pipeline and use log list
Output is then saved to pickle and can be further proceeded 
"""


class ResultsProvider():
    features = ['position',
                ]

    def __init__(self, mdf_data_provider, output_path, f_save_to_file=False, ds_filters=DS_FILTERS, kpi_filters=KPI_FILTERS, signal_feature_mapper=BMW_ASPE_SIGNAL_MAPPER,
                 kpi_mapper=BMW_ASPE_KPI_MAPPER, results_dir="", latency_mapper = ASPE_LATENCY_MAPPING,
                 logger_path="", ):
        self.mdf_data_provider = mdf_data_provider
        self.logger_path = logger_path
        self.results_dir = results_dir
        self.ds_filters = ds_filters
        self.kpi_filters = kpi_filters
        self.signal_feature_mapper = signal_feature_mapper
        self.kpi_mapper = kpi_mapper
        self.header = self.load_header_structure(output_path)
        self.f_save_to_file = f_save_to_file
        self.latency_mapper = latency_mapper
        self.log_list = []
        self.logs_paths = {}
        self.results = {}

    @staticmethod
    def load_header_structure(output_path, sheet_name="REQ"):
        df = pd.read_excel(output_path, sheet_name=sheet_name, header=None)
        df = df.loc[0:1, :].fillna(method='ffill', axis=1).append(df.loc[2:3, :])
        formated_df = df.rename(index={0: 'feature', 1: 'KPI', 2: 'interval', 3: 'value'})
        return formated_df

    def get_log_list(self, logs_dir):
        test_case_signatures = []
        for file in os.listdir(logs_dir):
            try:
                test_case_signatures.append(re.search(r'DS_\d+', file).group())
            except AttributeError as e:
                print(e)

        for test_case_signature in list(set(test_case_signatures)):
            self.logs_paths.update({test_case_signature: get_log_list_from_path(logs_dir, required_sub_strings=[test_case_signature], req_ext='.MF4', level=1)})

    def _get_filters(self):
        filters = deepcopy(self.header)
        for ds_name, rel_filters in self.ds_filters.items():
            filter_list = []
            for feature in self.header.loc['feature'].drop_duplicates():
                for kpi in self.header[self.header.columns[self.header.isin([feature]).any()]].loc['KPI'].drop_duplicates():
                    for interval in self.header[self.header.columns[self.header.isin([feature]).any()] & self.header.columns[self.header.isin([kpi]).any()]].loc['interval']:
                        try:
                            ref = rel_filters['ref'] + self.kpi_filters[feature][kpi][interval]
                        except KeyError:
                            ref = np.nan
                        est = rel_filters['est']
                        filter_list.append({'ref': ref, 'est': est})
            filters.loc[ds_name] = pd.Series(filter_list)
        return filters

    def evaluate(self, ds_filter, ds_name, filters):
        output = NestedDict()
        try:
            pe_output = self.process_loglist(self.ds_filters[ds_name], ds_name)
            output[ds_name]['full'] = pe_output
        except Exception as e:
            print(e)

        if self.f_save_to_file:
            save_to_pkl(pe_output, f'{self.results_dir}\\{ds_name}_pe_output.pickle')

        cached_intervals = {}

        single_ds_filters = list(ds_filter)
        for col in filters:
            single_column = filters[col]
            if isinstance(single_column[ds_name]['ref'], list):
                if single_column.interval not in cached_intervals.keys():
                    try:
                        pe_output = self.process_loglist(single_ds_filters[col], ds_name)
                        cached_intervals.update({single_column.interval: pe_output})
                        output[single_column.feature][single_column.KPI][single_column.interval] = pe_output
                    except Exception as e:
                        print(e)
                else:
                    output[single_column.feature][single_column.KPI][single_column.interval] = cached_intervals[single_column.interval]
                # if self.f_save_to_file:
                #     save_to_pkl(pe_output, f'{self.logs_dir}\\{ds_name}_{single_column.feature}_{single_column.interval}_pe_output.pickle')
        return output

    def process_loglist(self, relevancy_filters, ds_name):
        pe_output = []
        pipeline = PEPipeline(relevancy_estimated_objects=relevancy_filters['est'],
                              relevancy_reference_objects=relevancy_filters['ref'],
                              pairs_builder=PEPairedObjectsPreBuilderGating(distance_threshold=4.5),
                              association=RelEstToRefNN(assoc_distance_threshold=4.5),
                              pairs_features_kpis=EXTENDED_PAIRS_KPI_DICT, pairs_features_list=EXTENDED_PAIRS_FEATURES_LIST)
        multi_log_eval = PEMultiLogEvaluation(single_log_pipeline=pipeline,
                                              data_provider=self.mdf_data_provider,
                                              logging_file_folder=self.logger_path, pairs_features_kpis=EXTENDED_PAIRS_KPI_DICT)

        pe_output = multi_log_eval.process_data(self.logs_paths[ds_name])
        return pe_output

    def evaluate_all_scenarios(self):
        output = self.header.iloc[0:4, :]
        filters = self._get_filters()
        for rel_filter in tqdm(filters.iloc[4:, :].iterrows()):
            ds_name = rel_filter[0]
            ds_filters = rel_filter[1]
            if ds_name in self.logs_paths.keys():
                result = self.evaluate(ds_filters, ds_name, filters)
                kpi_list_for_ds = self.extract_results(result, ds_name)
                output.loc[ds_name] = kpi_list_for_ds
        return output

    def extract_results(self, result, ds_name):

        selected_kpi = []
        for feature in self.header.loc['feature'].drop_duplicates():
            for kpi in self.header[self.header.columns[self.header.isin([feature]).any()]].loc['KPI'].drop_duplicates():
                for interval in self.header[self.header.columns[self.header.isin([feature]).any()] & self.header.columns[self.header.isin([kpi]).any()]].loc['interval']:
                    if kpi == 'latency':
                        if interval != 'new':
                            single_result = result[ds_name]['full'].kpis_pairs_lifetime_aggregated
                            single_kpi = single_result[(single_result['feature_signature'] == self.latency_mapper[feature]) & (
                                    single_result['kpi_signature'] == 'STD')].kpi_value.squeeze()
                        else:
                            single_kpi = np.nan

                        selected_kpi.append(single_kpi)
                    elif isinstance(interval, str):
                        try:
                            single_result = result[feature][kpi][interval].kpis_pairs_features_aggregated
                            single_kpi = single_result[
                                (single_result.feature_signature == self.signal_feature_mapper[feature]) & (single_result.kpi_signature == self.kpi_mapper[kpi])].kpi_value
                            selected_kpi.append(single_kpi.squeeze())
                        except AttributeError or KeyError:
                            if interval in [self.ds_filters[ds_name]['direction'], self.ds_filters[ds_name]['straightness']]:
                                try:
                                    single_result = result[ds_name]['full'].kpis_pairs_features_aggregated
                                    single_kpi = single_result[(single_result.feature_signature == self.signal_feature_mapper[feature]) & (
                                            single_result.kpi_signature == self.kpi_mapper[kpi])].kpi_value
                                    selected_kpi.append(single_kpi.squeeze())
                                except KeyError:
                                    selected_kpi.append(np.nan)
                            else:
                                selected_kpi.append(np.nan)
                    else:
                        try:
                            if feature == 'object detection rate':
                                single_result = result[ds_name]['full'].kpis_binary_class_aggregated
                                single_kpi = single_result[single_result.signature == self.kpi_mapper[kpi]].value
                            else:
                                single_result = result[ds_name]['full'].kpis_pairs_features_aggregated
                                single_kpi = single_result[
                                    (single_result.feature_signature == self.signal_feature_mapper[feature]) & (single_result.kpi_signature == self.kpi_mapper[kpi])].kpi_value
                            selected_kpi.append(single_kpi.squeeze())
                        except KeyError:
                            selected_kpi.append(np.nan)
        filled = [np.nan if isinstance(x, pd.Series) else x for x in selected_kpi]
        return pd.Series(filled)


if __name__ == '__main__':
    logs_dir = r"Y:\02_FTP\20200619_A370_TRACKER\DS_03_old\BN_FASETH"
    logger_path = r'Y:\02_FTP\20200619_A370_TRACKER\DS_03_old\logger'
    sw_version = 'A370'  # 'A' step
    host_rear_axle_to_f_bumper = 3.7  # distance from ego vehicle rear axle to front bumper - need for CS transform
    host_length = 4.7
    host_width = 1.9
    cs_system = 'VCS'  # coordinate system in which evaluation will be performed
    rt_f360_dt = 0.047  # time delay between RtRange and F360 which must be compensated
    # time_synch_metod = ShiftSlaveExtractedDataTimestamp(offset=rt_f360_dt, inplace=True)

    mdf_data_provider = F360MdfRTRangeDataProvider(sw_version=sw_version,
                                                   host_rear_axle_to_front_bumper_dist=host_rear_axle_to_f_bumper,
                                                   host_width=host_width,
                                                   host_length=host_length,
                                                   coordinate_system=cs_system,
                                                   rt_hunter_target_shift=10,
                                                   # time_synch_metod=time_synch_metod,
                                                   save_to_file=True)

    provider = ResultsProvider(mdf_data_provider, r"template.xlsx", logger_path=logger_path, f_save_to_file=True, results_dir=logs_dir)
    provider.get_log_list(logs_dir)
    # reduced = reduce_loglist(provider.logs_paths, r'Y:\02_FTP\20200619_A370_TRACKER\DS_03\BN_FASETH')
    # provider.logs_paths = reduced
    result = provider.evaluate_all_scenarios()
    result.to_excel('KPI_report_DS_03_v2.xlsx')
    pass

