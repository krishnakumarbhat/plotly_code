from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.PairsFeaturesKPIManager import EXTENDED_PAIRS_KPI_DICT
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import EXTENDED_PAIRS_FEATURES_LIST
from aspe.evaluation.RadarObjectsEvaluation.PEMultiLogEvaluation import PEMultiLogEvaluation
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.providers.F360MdfRTRangeDataProvider import F360MdfRTRangeDataProvider
from aspe.providers.support_functions import get_log_list_from_path
from aspe.utilities.SupportingFunctions import save_to_pkl

"""
Run this script as 'Run File in Console'
Example of evaluating performance when several logs should be evaluated.
This class is configured by data provider and PE Pipeline and use log list
Output is then saved to pickle and can be further proceeded 
"""

logs_dir = r'C:\logs\VTV_mf4_logs\A370\BN_FASETH'
logger_path = 'C:\logs\VTV_mf4_logs\A370\BN_FASETH'
sw_version = 'A370'  # 'A' step
host_rear_axle_to_f_bumper = 3.7  # distance from ego vehicle rear axle to front bumper - need for CS transform
host_length = 4.7
host_width = 1.9
cs_system = 'VCS'  # coordinate system in which evaluation will be performed
rt_f360_dt = 0.047  # time delay between RtRange and F360 which must be compensated

mdf_data_provider = F360MdfRTRangeDataProvider(sw_version=sw_version,
                                               host_rear_axle_to_front_bumper_dist=host_rear_axle_to_f_bumper,
                                               host_width=host_width,
                                               host_length=host_length,
                                               coordinate_system=cs_system,
                                               reference_time_shift=rt_f360_dt,
                                               rt_hunter_target_shift=10,
                                               save_to_file=True)

test_case_signature = 'DS_01'
log_list = get_log_list_from_path(logs_dir, required_sub_strings=[test_case_signature], req_ext='.MF4',
                                  level=1)  # filter TestCaseX

multi_log_eval = PEMultiLogEvaluation(single_log_pipeline=PEPipeline(pairs_features_kpis=EXTENDED_PAIRS_KPI_DICT,
                                                                     pairs_features_list=EXTENDED_PAIRS_FEATURES_LIST),
                                      data_provider=mdf_data_provider,
                                      logging_file_folder=logger_path)

pe_output = multi_log_eval.process_data(log_list)
save_to_pkl(pe_output, f'{logs_dir}\\{test_case_signature}_pe_output.pickle')
