import os
import pickle

from aspe.evaluation.RadarObjectsEvaluation.KPI import DEFAULT_BIN_CLASS_KPI_LIST
from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.PairsFeaturesKPIManager import EXTENDED_PAIRS_KPI_DICT
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import EXTENDED_PAIRS_FEATURES_LIST
from aspe.evaluation.RadarObjectsEvaluation.PEMultiLogEvaluation import PEMultiLogEvaluation
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.extractors.ReferenceExtractor.RtRange.rt_3000_msg_sets_config import rt_3000_message_sets_config_ch_4
from aspe.providers.F360MudpRTRangeDataProvider import F360MudpRTRangeDataProvider
from aspe.providers.support_functions import get_log_list_from_path

"""
Run this script as 'Run File in Console'
Example of evaluating performance when several logs should be evaluated.
This class is configured by data provider and PE Pipeline and use log list
Output is then saved to pickle and can be further proceeded 
"""

mudp_stream_def_path = ExampleData.mudp_stream_def_path
current_dir = os.path.dirname(os.path.abspath(__file__))
dbc_config = {4: os.path.join(current_dir, r'configs\RT_range_target_merged_working.dbc')}

data_path = ExampleData.rt_range_data_path
logger_path = ExampleData.multi_log_pipeline_logging_folder
save_path_2_12 = ExampleData.multi_log_evaluation_results_2_12_path
save_path_2_15 = ExampleData.multi_log_evaluation_results_2_15_path

f360_data_provider = F360MudpRTRangeDataProvider(dbc_config, rt_3000_message_sets_config_ch_4, mudp_stream_def_path)

log_list_2_12 = get_log_list_from_path(data_path, required_sub_strings=['LSS', '2_12.'], level=4)
log_list_2_15 = get_log_list_from_path(data_path, required_sub_strings=['LSS', '2_15.'], level=4)

pe_pipeline = PEPipeline(pairs_features_list=EXTENDED_PAIRS_FEATURES_LIST,
                         pairs_features_kpis=EXTENDED_PAIRS_KPI_DICT,
                         objects_features_kpis=DEFAULT_BIN_CLASS_KPI_LIST)

pipeline_1 = PEMultiLogEvaluation(single_log_pipeline=pe_pipeline,
                                  data_provider=f360_data_provider,
                                  logging_file_folder=logger_path,
                                  pairs_features_kpis=EXTENDED_PAIRS_KPI_DICT)

pipeline_2 = PEMultiLogEvaluation(single_log_pipeline=pe_pipeline,
                                  data_provider=f360_data_provider,
                                  logging_file_folder=logger_path,
                                  pairs_features_kpis=EXTENDED_PAIRS_KPI_DICT)

pe_output_multi_log_2_12 = pipeline_1.process_data(log_list_2_12)
pe_output_multi_log_2_12.recalculate_scan_indexes()
pe_output_multi_log_2_15 = pipeline_2.process_data(log_list_2_15)
pe_output_multi_log_2_15.recalculate_scan_indexes()

with open(save_path_2_12, 'wb') as handle:
    pickle.dump(pe_output_multi_log_2_12, handle, protocol=pickle.HIGHEST_PROTOCOL)

with open(save_path_2_15, 'wb') as handle:
    pickle.dump(pe_output_multi_log_2_15, handle, protocol=pickle.HIGHEST_PROTOCOL)
