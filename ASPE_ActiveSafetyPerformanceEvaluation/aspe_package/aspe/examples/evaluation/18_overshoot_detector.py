from pathlib import Path

from aspe.evaluation.RadarObjectsEvaluation.PairsLifetimeFeatures.Overshoot import Overshoot
from aspe.evaluation.RadarObjectsEvaluation.PEMultiLogEvaluation import PEMultiLogEvaluation
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.providers.F360MudpObjectAndSensorDP import F360MudpObjectAndSensorDP
from aspe.providers.support_functions import get_log_list_from_path
from aspe.utilities.SupportingFunctions import save_to_pkl

"""
Run this script as 'Run File in Console'
Example of single log evaluation:
pe - performance evaluation pipeline is configured by defaults strategies

The script is used to detect overshoots of objects acceleration and velocity. Output is given by overshoot magnitude 
and duration.

Provide unique_id of objects which need to be evaluate via overshoot detector. If the list is empty, all objects will be
taken into account.
"""
objects_to_evaluate = [] #unique_id only
multi_log_eval = True  # Set true if you want to perform multilog evaluation
test_case_signature = ''  # leave as empty string if whole logs_dir folder should be evaluated, ONLY FOR MULTI LOG
logs_dir = ExampleData.analysis_data_path + r"\overshoot_detector" # File path for single log, directory for multi log



mudp_stream_defs_dir = ExampleData.mudp_stream_def_path
data_provider = F360MudpObjectAndSensorDP(mudp_stream_def_path=mudp_stream_defs_dir)
pe = PEPipeline(pairs_lifetime_features=[Overshoot(objects_to_evaluate=objects_to_evaluate)],
                ref_from_internal_objects=True,
                evaluate_internal_objects=True,)

if multi_log_eval:
    pe_multi_log = PEMultiLogEvaluation(single_log_pipeline=pe,
                                        data_provider=data_provider,
                                        logging_file_folder=logs_dir)
    log_list = get_log_list_from_path(logs_dir,
                                      required_sub_strings=[test_case_signature],
                                      req_ext='.mudp',
                                      level=0)  # filter TestCaseX
    pe_output = pe_multi_log.process_data(log_list)
    if test_case_signature == '':
        test_case_signature = Path(logs_dir).stem
    save_to_pkl(pe_output, f'{logs_dir}\\{test_case_signature}_pe_output.pickle')

else:
    estimated_data, reference_data = data_provider.get_single_log_data(logs_dir)
    pe_output = pe.evaluate(estimated_data, estimated_data)

    save_to_pkl(pe_output, f'{logs_dir}_pe_output.pickle')
