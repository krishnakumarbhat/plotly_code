from PerformanceEvaluation.PerformanceEvaluationWrapper.WrapperScript import PerformanceEvaluationWrapper
import os
import pandas as pd
import pickle
from typing import List

CONFIG_PATH = r"configs\wrapper_config.json"


def run_evaluation_for_scenario(root_folder_path, folder_phrases_req: List[str], log_name_phrase_restricted: str):
    aggregated_data_paths = []
    for (root, dirs, files) in os.walk(root_folder_path):
        f_phrase_is_in_folder_path = [phrase in root for phrase in folder_phrases_req]
        if all(f_phrase_is_in_folder_path):
            log_paths_list = [os.path.join(root, log_name) for log_name in files if '.dvl' in log_name and
                              log_name_phrase_restricted not in log_name]
            align_angle = get_align_angle_value_from_root_path(root)
            agr_data_path = run_evaluation_on_log_list(align_angle, log_paths_list, root)
            if agr_data_path is not None:
                aggregated_data_paths.append(agr_data_path)
    return aggregated_data_paths


def get_align_angle_value_from_root_path(root_path: str):
    align_angle_str = root_path.split('_align_cor_')[1].replace('_', '.')
    if 'neg.' in align_angle_str:
        align_angle_str = align_angle_str.replace('neg.', '-')
    align_angle = float(align_angle_str)
    return align_angle


def run_evaluation_on_log_list(align_angle: float, log_paths_list: List[str], root_path:str):
    pe_wrapper = PerformanceEvaluationWrapper(CONFIG_PATH)
    corrupted_logs = []
    agr_data_path = None
    log_count = len(log_paths_list)
    for log_idx, log_path in enumerate(log_paths_list):
        print('-----------------------------------------')
        print(f'PROCESSING LOG NUMBER: {log_idx} / {log_count}')
        print(f'Log path: {log_path}')
        try:
            pe_wrapper.process_single_log(log_path, log_idx)
        except Exception as exc:
            corrupted_logs.append((log_idx, log_path))
            print(exc)
    try:
        agr_data = get_aggregated_data(pe_wrapper, align_angle)
        agr_data_path = save_agr_data(agr_data, root_path)
    except Exception as exc:
        print(f'Aggregation failed for :{root_path}\n')
        print(exc)
    return agr_data_path


def get_aggregated_data(pe_wrapper: PerformanceEvaluationWrapper, align_angle: float):
    ref_data = pd.concat(pe_wrapper.ref_data_frames, ignore_index=True)
    est_data = pd.concat(pe_wrapper.est_data_frames, ignore_index=True)
    agg_data = {
        'reference_obj_data': ref_data,
        'estimated_obj_data': est_data,
        'log_data': pe_wrapper.aggregated_log_data,
        'host_data': pe_wrapper.aggregated_host_data,
        'align_angle': align_angle
    }
    return agg_data


def save_agr_data(agr_data: dict, root_path: str):
    _, folder_name = os.path.split(root_path)
    save_file_name = folder_name + '_aggregated_data.pickle'
    save_file_path = os.path.join(root_path, save_file_name)
    with open(save_file_path, 'wb') as file:
        pickle.dump(agr_data, file)
    return save_file_path


if __name__ == '__main__':
    scenario_folder_path = r"P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\LSS_FTP_201"
    agg_data_paths = run_evaluation_for_scenario(scenario_folder_path, ['_align_', '_case_2'], '_TC4_')

    with open(os.path.join(scenario_folder_path, 'aggregated_PE_data_paths.txt'), 'w') as file:
        file.writelines('\n'.join(agg_data_paths))

    scenario_folder_path = r"P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA_FTP_402"
    agg_data_paths = run_evaluation_for_scenario(scenario_folder_path, ['_align_', '_case_2'], '_TC4_')

    with open(os.path.join(scenario_folder_path, 'aggregated_PE_data_paths.txt'), 'w') as file:
        file.writelines('\n'.join(agg_data_paths))