import pandas as pd
import numpy as np
from analysis_scripts.DEX80_missalignment_analysis.utils import save_pickle_file, load_pickle_file

NEEDED_COLUMNS = [
    'log_idx',
    'obj_idx',
    'scan_index',
    'position_vcsx',
    'position_vcsy',
    'vcs_heading',
    'velocity_vcsx',
    'velocity_vcsy',
    'relevancy_flag',
    'binary_classification',
    'associated_with',
    'deviation_pos_vcsx',
    'deviation_pos_vcsy',
    'deviation_vel_vcsx',
    'deviation_vel_vcsy']


def extract_save_concated_data(agg_data_paths, out_save_path):
    paths_count = len(agg_data_paths)
    est_data_frames = []
    ref_data_frames = []
    for path_idx, single_path in enumerate(agg_data_paths):
        print(f'Processing file number: {path_idx} / {paths_count}')
        agg_data = load_pickle_file(single_path)
        est_data_raw = agg_data['estimated_obj_data']
        ref_data_raw = agg_data['reference_obj_data']
        align_angle = agg_data['align_angle']

        # FILTER NEEDED SIGNALS
        est_data_filtered = est_data_raw[NEEDED_COLUMNS + ['vcs_pointing_angle']]
        ref_data_filtered = ref_data_raw[NEEDED_COLUMNS]

        # ADD MISALIGNMENT ANGLE VAL
        est_data_filtered.loc[:, 'align_angle'] = align_angle
        ref_data_filtered.loc[:, 'align_angle'] = align_angle
        process_columns(est_data_filtered)
        process_columns(ref_data_filtered)
        ref_data_filtered.loc[:, 'vcs_pointing_angle'] = np.deg2rad(ref_data_filtered.loc[:, 'vcs_heading'].values)

        add_range_signal(est_data_filtered)
        add_range_signal(ref_data_filtered)

        add_deviations_in_tcs(ref_data_filtered)

        est_data_frames.append(est_data_filtered)
        ref_data_frames.append(ref_data_filtered)

    est_data_concated = pd.concat(est_data_frames)
    ref_data_concated = pd.concat(ref_data_frames)

    output = {'estimated_obj_data': est_data_concated,
              'reference_obj_data': ref_data_concated,
              'log_data': agg_data['log_data'],
              'host_data': agg_data['host_data']}
    save_pickle_file(output, out_save_path)


def add_range_signal(df):
    df.loc[:, 'range'] = np.hypot(df.loc[:, 'position_vcsx'].values, df.loc[:, 'position_vcsy'].values)


def add_deviations_in_tcs(ref_obj_df):
    dev_pos_x = ref_obj_df.loc[:, 'deviation_pos_vcsx']
    dev_pos_y = ref_obj_df.loc[:, 'deviation_pos_vcsy']
    dev_vel_x = ref_obj_df.loc[:, 'deviation_vel_vcsx']
    dev_vel_y = ref_obj_df.loc[:, 'deviation_vel_vcsy']

    target_pointing = ref_obj_df.loc[:, 'vcs_pointing_angle']
    ref_obj_df.loc[:, 'deviation_pos_tcsx'] = dev_pos_x * np.cos(-target_pointing) - dev_pos_y * np.sin(-target_pointing)
    ref_obj_df.loc[:, 'deviation_pos_tcsy'] = dev_pos_x * np.sin(-target_pointing) + dev_pos_y * np.cos(-target_pointing)

    ref_obj_df.loc[:, 'deviation_vel_tcsx'] = dev_vel_x * np.cos(-target_pointing) - dev_vel_y * np.sin(-target_pointing)
    ref_obj_df.loc[:, 'deviation_vel_tcsy'] = dev_vel_x * np.sin(-target_pointing) + dev_vel_y * np.cos(-target_pointing)


def read_agg_data_paths_from_txt_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    paths = [line.replace('\n', '')for line in lines]
    return paths


def process_columns(df):
    rel_col = df['relevancy_flag'].values
    rel_col_new = np.full(rel_col.shape[0], False)
    relevant_mask = rel_col == 'RL'
    rel_col_new[relevant_mask] = True
    df.loc[:, 'relevancy_flag'] = rel_col_new


def get_reference_rel_speed(ref_df, host_df):
    ref_df.reset_index(inplace=True)
    ref_df['old_idx'] = ref_df.index
    ref_df['rel_speed'] = np.nan
    for (log_idx, ref_log_df), (log_idx_host, host_log_df) in zip(ref_df.groupby(by=['log_idx']), host_df.groupby(by='log_idx')):
        ref_log_2 = ref_log_df.set_index('scan_index', drop=False)
        host_speed = host_log_df.set_index('scan_idx', drop=False)['speed']

        ref_log = ref_log_2.join(host_speed)
        ref_log['rel_speed'] = ref_log['velocity_vcsx'] - ref_log['speed']
        idxs = ref_log['old_idx'].values
        ref_df.loc[idxs, 'rel_speed'] = ref_log['rel_speed'].values


if __name__ == '__main__':
    agg_data_txt_paths = [r"P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA_FTP_402\case_2_aggregated_PE_data_paths.txt"]
    data_save_paths = [r"C:\projects\F360\DEX80_misalign_analysis_data_RCTA_case_2.pickle"]

    for input_txt_path, out_save_path in zip(agg_data_txt_paths, data_save_paths):
        agg_data_paths = read_agg_data_paths_from_txt_file(input_txt_path)
        extract_save_concated_data(agg_data_paths, out_save_path)

