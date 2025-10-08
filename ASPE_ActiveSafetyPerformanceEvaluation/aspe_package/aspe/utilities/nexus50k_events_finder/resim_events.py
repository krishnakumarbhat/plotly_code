import re
import subprocess
from collections import defaultdict
from datetime import datetime, timedelta
from multiprocessing import Pool, Value
from pathlib import Path
from typing import Iterable, List

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluation import Nexus50kEvaluation
from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.utilities.nexus50k_events_finder.pe_piepelines_setup import get_default_pipeline_for_50k_eval
from aspe.utilities.nexus50k_events_finder.user_config import user_config
from aspe.utilities.nexus50k_events_finder.utils import get_events_list_from_csv

'''
RESIM_COUNTER variable and init function are needed for tracking current number of processed log list. Needs to be 
defined in that way to work with multiple processes.
'''

RESIM_COUNTER = Value('i', 0)


def init(args):
    global RESIM_COUNTER
    RESIM_COUNTER = args


def resimulate_events_logs(config: Nexus50kEvaluationConfig,
                           events_to_resim_csv_list: List[str],
                           resim_exe_path: str,
                           resim_osuffix: str,
                           n_processes=6,
                           only_n_first_events=None):

    RESIM_COUNTER.value = 0
    pe_pipeline = get_default_pipeline_for_50k_eval()
    evaluation = Nexus50kEvaluation(config=config, pe_pipeline=pe_pipeline)

    log_list_paths = create_resim_log_lists_for_events(events_to_resim_csv_list, evaluation,
                                                       only_n_first_events=only_n_first_events)

    with Pool(processes=n_processes, initializer=init, initargs=(RESIM_COUNTER, )) as pool:
        args = [(resim_exe_path, resim_osuffix, l, len(log_list_paths)) for l in log_list_paths]
        out_msgs = pool.starmap(run_resim_for_log_list, args)

    # write resim console out messages to .txt log file - can be usefull if output files are corrupted
    now_str = datetime.now().strftime("%Y%m%d_%H%M%S")
    with open(Path(config.SRR_DEBUG_CACHE_DIR) / f'resim_run_log_{now_str}.txt', mode='w') as f:
        for msg in out_msgs:
            f.write('--------------------------------------------------------------------------')
            f.write(msg)
    return out_msgs


def run_resim_for_log_list(resim_exe_path: str, osuffix: str, log_list_path: str, total_count: int):

    global RESIM_COUNTER
    with RESIM_COUNTER.get_lock():
        RESIM_COUNTER.value += 1

    print(f'started resim! {RESIM_COUNTER.value} / {total_count}')
    command = f'{resim_exe_path} {log_list_path} -filelist -osuffix {osuffix} -stream BMW -f360trkopt -sync_input -init_from_log -endopt'
    output = subprocess.getoutput(command)
    return output


def create_resim_log_lists_for_events(events_to_resim_csv: List[str],
                                      evaluation: Nexus50kEvaluation,
                                      only_n_first_events=None) -> List[str]:

    events_list = [ev for p in events_to_resim_csv for ev in get_events_list_from_csv(p, only_n_first_events)]
    srr_debug_list_per_nexus_log = defaultdict(lambda: set())

    for ev in events_list:
        ev.tmin -= timedelta(seconds=20)  # for resim purposes we want to start resimulation before event occurance
        event_df = evaluation.find_event_in_nexus_to_hpc_table(ev)
        logs = event_df.hpcc_path.to_list()
        srr_debug_list_per_nexus_log[ev.nexus_log_id].update(logs)

    logs_dfs = []
    for k, v in srr_debug_list_per_nexus_log.items():
        srr_debug_list_per_nexus_log[k] = sorted(list(v))
        logs_dfs.append(pd.DataFrame({'hpcc_path': srr_debug_list_per_nexus_log[k], 'nexus_log_id': k}))

    logs_df = pd.concat(logs_dfs).reset_index(drop=True)
    mudp_paths = evaluation.download_mf4_and_convert_to_mudp_event_logs(logs_df)

    logs_df['mudp_paths'] = mudp_paths

    log_list_paths = []
    for nexus_id, group in logs_df.groupby(by='nexus_log_id'):
        mudp_group_paths = group.loc[:, 'mudp_paths']
        log_list_paths.extend(
            create_temp_cont_resim_log_lists(mudp_group_paths, evaluation.config, nexus_id))
    return log_list_paths


def create_temp_cont_resim_log_lists(log_list, config: Nexus50kEvaluationConfig, nexus_id: str) -> List[str]:
    log_list_dir = Path(config.DATA_50K_CACHE_DIR) / 'resim_log_lists'
    log_list_dir.mkdir(exist_ok=True, parents=True)
    log_lists_paths = []

    log_list_count = 0
    grouped = split_log_list_into_continous_chunks(log_list)
    for group in grouped:
        temp_log_list_path = log_list_dir / f'temp_resim_log_list_{nexus_id}_{log_list_count}.txt'
        with open(temp_log_list_path, 'w') as file:
            for pat in group:
                file.write(f"{pat}\n")
        log_lists_paths.append(temp_log_list_path)
        log_list_count += 1
    return log_lists_paths


def split_log_list_into_continous_chunks(log_list: Iterable[str]):
    logs_numbers = [int(re.search('deb_(.+?).mudp', p).group(1)) for p in sorted(log_list)]
    split_indexes = np.where(np.diff(logs_numbers) > 1)[0] + 1
    return np.split(log_list, split_indexes)


if __name__ == '__main__':
    events_to_resim_csv = [
        r"E:\logfiles\nexus_50k_eval_data\overtaking_events.csv",
        r"E:\logfiles\nexus_50k_eval_data\close_trucks_events.csv",
        r"E:\logfiles\nexus_50k_eval_data\close_pedestrians_events.csv",
        r"E:\logfiles\nexus_50k_eval_data\stop_and_go_events.csv",
    ]

    n_processes = 6

    resims = {
        r"E:\resims\DEX_2503_base_5ffcba9\resim_f360.exe": '_DEX_2503_base_5ffcba9',
        r"E:\resims\DEX_2503_occ_in_init_e8a9e77\resim_f360.exe": '_DEX_2503_occ_in_init_e8a9e77',
        r"E:\resims\DEX_2503_lock_conf_until_dets_visible_d18cd14\resim_f360.exe": '_DEX_2503_lock_conf_until_dets_visible_d18cd14',
    }

    for resim_exe_path, resim_osuffix in resims.items():
        out = resimulate_events_logs(user_config,
                                     events_to_resim_csv,
                                     resim_exe_path,
                                     resim_osuffix,
                                     n_processes,
                                     only_n_first_events=None)


