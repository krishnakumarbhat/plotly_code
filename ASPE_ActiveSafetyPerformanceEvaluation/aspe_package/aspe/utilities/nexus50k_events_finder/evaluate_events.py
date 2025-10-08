from typing import List, Optional, Union

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluation import Nexus50kEvaluation, NexusEvent
from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.utilities.nexus50k_events_finder.pe_piepelines_setup import (
    get_default_pipeline_for_50k_eval,
    get_pe_pipeline_for_event_type,
)
from aspe.utilities.nexus50k_events_finder.user_config import user_config


def evaluate_events_set(config: Nexus50kEvaluationConfig,
                        events_set_source: Union[NexusEvent, List[NexusEvent], str, List[str]],
                        pe_pipeline: PEPipeline = None,
                        resim_exe_path: str = None,
                        resim_suffix: str = None,
                        resim_mydir: str = None,
                        existing_resimulated_output_dir_name: str = None,
                        single_events_output_save_dir: Optional[str] = None,
                        aggregated_output_save_dir: Optional[str] = None):
    """
    Run evaluation with set of events.
    :param config: evaluation config
    :param events_set_source: parameter defining group of events for evaluation. Can be passed in few forms:
                              1. NexusEvent - evaluate just single event
                              2. list of NexusEvent - evaluate in loop every event in loop
                              3. string - name of event group stored in Nexus (as EventName type) - evaluate all event
                                 from this group
                              4. string - path to .csv file which stores events definitions (output of events finder)
                              5. list of strings - each element contains Nexus id of event which needs to be evaluated
    :param pe_pipeline: performance evaluation pipeline, if not passed predefined pipeline for given events type will
    be used - check pe_pipelines_setup.py file
    :param resim_exe_path: path to F360 core resim .exe file whih will be used for resimulation of events logs
    :param resim_suffix: 'osuffix' resim parameter
    :param existing_resimulated_output_dir_name: name of directory with already resimulated files. Use this if logs
    which need to be evaluated are already resimed. If this argument is passed resim_exe_path, resim_suffix and
    resim_mydir are not used
    :param resim_mydir: 'mydir' resim parameter - defines directory where resimmed logs should be placed
    :param single_events_output_save_dir: directory where evaluation output for each single event will be saved
    :param aggregated_output_save_dir: directory where aggregated results for all events should be saved
    :return: None
    """
    if pe_pipeline is None:
        pe_pipeline = get_pe_pipeline_for_event_type(events_set_source)
    evaluation = Nexus50kEvaluation(config=config, pe_pipeline=pe_pipeline)
    evaluation.evaluate_f360_resim_with_nexus_events_set(events_source=events_set_source,
                                                         resim_exe_path=resim_exe_path,
                                                         resim_suffix=resim_suffix,
                                                         resim_mydir=resim_mydir,
                                                         existing_resimulated_output_dir_name=existing_resimulated_output_dir_name,
                                                         single_event_output_save_dir=single_events_output_save_dir,
                                                         aggregated_output_save_dir=aggregated_output_save_dir)


if __name__ == '__main__':
    events_csv_path = r"E:\logfiles\nexus_50k_eval_data\close_trucks_events.csv"
    resims_to_eval = ['rRf360t7060309v205p50_dev_baseline',
                      #'rRf360t7010309v205p50_DEX_2503_occ_in_init_e8a9e77',
                      #'rRf360t7010309v205p50_DEX_2503_lock_conf_until_dets_visible_d18cd14'
                      ]
    user_config = Nexus50kEvaluationConfig(main_cache_dir=r"E:\logfiles\nexus_50k_eval_data")
    for resim_name in resims_to_eval:
        evaluate_events_set(config=user_config,
                            events_set_source='f360_close_trucks',
                            existing_resimulated_output_dir_name=resim_name,
                            aggregated_output_save_dir=user_config.PERF_EVAL_OUT_DIR)
