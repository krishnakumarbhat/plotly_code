from pathlib import Path

from aspe.evaluation.RadarObjectsEvaluation.PEMultiLogEvaluation import PEMultiLogEvaluation
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.providers.F360MudpGTfromDetsProvider import F360MudpGTfromDetsProvider
from aspe.providers.support_functions import get_log_list_from_path
from aspe.utilities.SupportingFunctions import save_to_pkl

"""
Example of evaluating performance when several logs should be evaluated, also allows to analyze single log.
This class is configured by data provider and PE Pipeline and use log list.
Detections based ground truth is created only for moving objects, unless you provide unique id of stationary objects.
Then also ground truth for objects with given unique ids is created. Static unique ids should be given as list in 
dictionary in order of logs in folder. Provider allows to setup some of ground truth script parameters (more in 
provider class description). 
Output is saved to pickle and can be further proceeded.
"""

multi_log_eval = True  # Set true if you want to perform multilog evaluation
logs_dir = ExampleData.analysis_data_path + r"\gt_from_dets"  # File path for single log, directory for multi log
test_case_signature = ''  # leave as empty string if whole logs_dir folder should be evaluated, ONLY FOR MULTI LOG

# Static objects ids to calculate ground truth
stat_u_ids = {
    'Ego40_0_Target0': [283],
    'Ego50_0_Target0': [365, 378],
    'Ego50_0_Target40': [],
    'Ego50_0_Target50': [],
    'Ego50_0_Target60': [],
    'Ego60_0_Target0': [187, 153],
    'Ego0_Target60': [],
}


mudp_stream_defs_dir = ExampleData.mudp_stream_def_path
# Create data provider
provider = F360MudpGTfromDetsProvider(
    mudp_stream_defs_path=mudp_stream_defs_dir,
    save_to_file=True,
    force_extract=True,
    stat_u_ids=stat_u_ids,
)

if multi_log_eval:

    pipeline = PEPipeline(evaluate_internal_objects=True)
    multi_log_eval = PEMultiLogEvaluation(single_log_pipeline=pipeline,
                                          data_provider=provider,
                                          logging_file_folder=logs_dir)
    log_list = get_log_list_from_path(logs_dir, required_sub_strings=[test_case_signature], req_ext='.mudp',
                                      level=0)  # filter TestCaseX
    pe_output = multi_log_eval.process_data(log_list)
    if test_case_signature == '':
        test_case_signature = Path(logs_dir).stem
    save_to_pkl(pe_output, f'{logs_dir}\\{test_case_signature}_pe_output.pickle')

else:
    pipeline = PEPipeline(evaluate_internal_objects=True)

    estimated_data, reference_data = provider.get_single_log_data(logs_dir)

    pe_output = pipeline.evaluate(estimated_data, reference_data)

    save_to_pkl(pe_output, f'{logs_dir}_pe_output.pickle')
