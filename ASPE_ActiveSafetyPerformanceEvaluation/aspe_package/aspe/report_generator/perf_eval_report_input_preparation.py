import pandas as pd
import numpy as np
from aspe.utilities.SupportingFunctions import load_from_pkl, save_to_pkl
from aspe.extractors.Interfaces.IDataSet import IDataSet
from collections import defaultdict
from aspe.utilities.SupportingFunctions import concat_dict_of_lists_of_dataframes


def load_perf_eval_results(pe_outputs_paths):
    atrs_to_concat = [
        'kpis_binary_class_aggregated',
        'kpis_pairs_features_aggregated',
        'pe_results_obj_pairs',
        'pe_results_obj_ref',
        'pe_results_obj_est'
    ]
    dict_of_dfs = defaultdict(list)
    for sw_version, test_cases in pe_outputs_paths.items():
        for test_case, tc_pickle_path in test_cases.items():
            pe_output = load_from_pkl(tc_pickle_path)
            add_reference_position_and_yaw_rate_signals_to_pairs_features(pe_output)
            add_reference_position_to_pe_ref_results(pe_output)
            add_host_yaw_rate_to_pairs_features(pe_output)
            convert_angular_features_to_degrees(pe_output)
            for att_name in atrs_to_concat:
                atr = get_atribute_from_pe_outtput(pe_output, att_name)
                if isinstance(atr, pd.DataFrame):
                    atr['subset'] = test_case
                    atr['sw_version'] = sw_version
                    dict_of_dfs[att_name].append(atr)
    return concat_dict_of_lists_of_dataframes(dict_of_dfs)


def get_atribute_from_pe_outtput(pe_output, att_name):
    try:
        atr = pe_output.__getattribute__(att_name)
        if isinstance(atr, IDataSet):
            atr = atr.signals
        return atr
    except AttributeError:
        print(f'Atribute {att_name} not available')
        return None


def add_reference_position_and_yaw_rate_signals_to_pairs_features(pe_output):
    ref_index = pe_output.pe_results_obj_pairs.signals.index_ref
    ref_pos = pe_output.extracted_reference_objects.signals.loc[ref_index, ['position_x', 'position_y', 'yaw_rate']].reset_index(drop=True)
    ref_pos = ref_pos.rename(columns={'position_x': 'ref_position_x', 'position_y': 'ref_position_y', 'yaw_rate': 'ref_yaw_rate'})
    ref_pos.loc[:, 'distance_to_ref_obj'] = np.hypot(ref_pos.ref_position_x, ref_pos.ref_position_y)
    pe_output.pe_results_obj_pairs.signals = pe_output.pe_results_obj_pairs.signals.join(ref_pos)


def add_reference_position_to_pe_ref_results(pe_output):
    ref_pos = pe_output.extracted_reference_objects.signals.loc[:, ["position_x", "position_y"]]
    ref_pos['distance_to_ref_obj'] = np.hypot(ref_pos['position_x'], ref_pos['position_y'])
    pe_output.pe_results_obj_ref.signals = pe_output.pe_results_obj_ref.signals.join(ref_pos)


def add_host_yaw_rate_to_pairs_features(pe_output):
    host_data = pe_output.extracted_reference_host.signals.loc[:, ['log_index', 'scan_index', 'yaw_rate']].rename(columns={'yaw_rate': 'host_yaw_rate'})
    pe_output.pe_results_obj_pairs.signals = pe_output.pe_results_obj_pairs.signals.join(host_data.set_index(['log_index', 'scan_index']), on=['log_index', 'scan_index'])


def convert_angular_features_to_degrees(pe_output):
    pe_output.pe_results_obj_pairs.signals['dev_yaw_rate'] = np.rad2deg(pe_output.pe_results_obj_pairs.signals['dev_yaw_rate'])
    pe_output.pe_results_obj_pairs.signals['dev_orientation'] = np.rad2deg(pe_output.pe_results_obj_pairs.signals['dev_orientation'])
