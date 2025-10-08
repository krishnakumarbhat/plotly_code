from pathlib import Path

from aspe.evaluation.RadarObjectsEvaluation.KPI.SeriesKPI.PairsFeaturesKPIManager import EXTENDED_PAIRS_KPI_DICT
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures import EXTENDED_PAIRS_FEATURES_LIST
from aspe.evaluation.RadarObjectsEvaluation.PEMultiLogEvaluation import PEMultiLogEvaluation
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.providers.F360MudpRtRangeMdfDataProvider import F360MudpRtRangeMdfDataProvider
from aspe.providers.support_functions import get_log_list_from_path
from aspe.utilities.SupportingFunctions import save_to_pkl

"""
Run this script as 'Run File in Console'
Example of evaluating performance when several logs should be evaluated.
This class is configured by data provider and PE Pipeline and use log list
Output is then saved to pickle and can be further proceeded 
"""

mudp_stream_defs_dir = "C:\wkspaces_git\F360Core\sw\zResimSupport\stream_definitions"
logs_dir = r'C:\logs\bbox_managment\SRR_DEBUG\rRf360t4280309v205p50_resize_fix'
test_case_signature = 'DS_12'  # leave as empty string if whole logs_dir folder should be evaluated

provider = F360MudpRtRangeMdfDataProvider(
    mudp_stream_defs_path=mudp_stream_defs_dir,
    rt_hunter_target_shift=10,
    reference_time_shift=-0.05,
    save_to_file=True,
    force_extract=True,
)

pipeline = PEPipeline(pairs_features_kpis=EXTENDED_PAIRS_KPI_DICT,
                      pairs_features_list=EXTENDED_PAIRS_FEATURES_LIST,
                      evaluate_internal_objects=True)

multi_log_eval = PEMultiLogEvaluation(single_log_pipeline=pipeline,
                                      data_provider=provider,
                                      logging_file_folder=logs_dir)

log_list = get_log_list_from_path(logs_dir, required_sub_strings=[test_case_signature], req_ext='.mudp',
                                  level=0)  # filter TestCaseX

pe_output = multi_log_eval.process_data(log_list)
if test_case_signature == '':
    test_case_signature = Path(logs_dir).stem
save_to_pkl(pe_output, f'{logs_dir}\\{test_case_signature}_pe_output.pickle')
