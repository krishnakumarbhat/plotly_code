from analysis_scripts.DEX80_missalignment_analysis.utils import load_pickle_file, save_pickle_file
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os

SAVE_FIG_FOLDER = r'P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\Analysis\analysis_plots'
CRITICAL_ERROR_POS_Y = 2.0
CRITICAL_ERROR_POS_X = 2.0
CRITICAL_ERROR_VEL_Y = 1.5
CRITICAL_ERROR_VEL_X = 1
DEFAULT_PLOT_SIZE = (10, 7.5)


def get_far_ranges_deviations_plots(ref_df_raw: pd.DataFrame, deviation_column, range_low, range_high, title, crit_error_val):
    far_ranges_mask = np.logical_and(range_low < ref_df_raw.range.values, ref_df_raw.range.values < range_high)
    ref_data_far_range = ref_df_raw.loc[far_ranges_mask, :]
    bias_vec = []
    bias_and_std3_vec = []
    angles_vec = []
    for align_angle, df in ref_data_far_range.groupby(by='align_angle'):
        valid_idxs = []
        for log_idx, log_df in df.groupby(by='log_idx'):

            # for RCTA scenario we want to filter out objects which passed VCSY = 0 line
            df_idxs = log_df.index.values
            for n in range(1, df_idxs.shape[0]-1):
                curr_idx = df_idxs[n]
                next_idx = df_idxs[n + 1]
                if log_df.loc[curr_idx, 'position_vcsy'] * log_df.loc[next_idx, 'position_vcsy'] < 0:
                    break
                valid_idxs.append(curr_idx)
                if n + 1 == df_idxs.shape[0]:
                    valid_idxs.append(next_idx)

        dev_signal = df.loc[valid_idxs, deviation_column].values
        dev_signal = dev_signal[~np.isnan(dev_signal)]
        abs_bias = np.abs(np.mean(dev_signal))
        std = np.quantile(np.abs(dev_signal - np.mean(dev_signal)), 0.682)
        bias_vec.append(abs_bias)
        bias_and_std3_vec.append(3*std)
        angles_vec.append(align_angle)

    fig, ax = plt.subplots(figsize=DEFAULT_PLOT_SIZE)
    bias_line, = ax.plot(angles_vec, bias_vec, label='BIAS', linestyle=':', color='red')
    std_line, = ax.plot(angles_vec, bias_and_std3_vec, label='BIAS + 3 STD', linestyle='-', color='red')
    crit_error_line, = ax.plot(angles_vec, [crit_error_val] * len(angles_vec), label='deviation threshold', color='blue')
    ax.set_xlabel('yaw angle shift [deg]')
    if 'pos' in deviation_column:
        ax.set_ylabel('deviation [m]')
    if 'vel' in deviation_column:
        ax.set_ylabel('deviation [m/s]')
    ax.set_title(title)
    ax.xaxis.set_ticks(np.arange(-3, 3.5, 0.5))
    plt.grid()
    plt.legend(handles=[bias_line, std_line, crit_error_line])
    file_name = f"{title.split()[-1]}_{deviation_column}_range_{range_low}_{range_high}.png"
    save_path = os.path.join(SAVE_FIG_FOLDER, file_name)
    fig.savefig(save_path)


def get_deviations_plots_in_range_fun(ref_df_raw: pd.DataFrame, deviation_column, title, crit_error_val):
    results_bias = {}
    results_3_std = {}
    range_bins = [(low * 5.0, (low + 1) * 5.0) for low in range(1, 12)]
    bin_centers = [(bin[0] + bin[1])/2 for bin in range_bins]
    for align_angle, df in ref_df_raw.groupby(by='align_angle'):
        bias_vec = []
        bias_and_std3_vec = []

        valid_idxs = []
        for log_idx, log_df in df.groupby(by='log_idx'):
            df_idxs = log_df.index.values
            for n in range(1, df_idxs.shape[0] - 1):
                curr_idx = df_idxs[n]
                next_idx = df_idxs[n + 1]
                if log_df.loc[curr_idx, 'position_vcsy'] * log_df.loc[next_idx, 'position_vcsy'] < 0:
                    break
                valid_idxs.append(curr_idx)
                if n + 1 == df_idxs.shape[0]:
                    valid_idxs.append(next_idx)

        if align_angle == 0.0:
            out_ref_data_filtered = df.loc[valid_idxs, :]  # output for trace plots, we want to plot already processed data
        for bin in range_bins:
            df_filtered = df.loc[valid_idxs, :]
            range_mask = np.logical_and(bin[0] < df_filtered.range.values, df_filtered.range.values < bin[1])
            dev_signal = df_filtered.loc[range_mask, deviation_column].values
            dev_signal = dev_signal[~np.isnan(dev_signal)]

            abs_bias = np.abs(np.mean(dev_signal))
            std = np.quantile(np.abs(dev_signal - np.mean(dev_signal)), 0.682)

            bias_vec.append(abs_bias)
            bias_and_std3_vec.append(abs_bias + 3*std)
        results_bias[align_angle] = bias_vec
        results_3_std[align_angle] = bias_and_std3_vec

    fig, ax = plt.subplots(figsize=DEFAULT_PLOT_SIZE)

    line_1, = ax.plot(bin_centers, results_bias[0.0], label='0 deg y.a.s. - BIAS', ls=':', color='black')
    line_2, = ax.plot(bin_centers, results_3_std[0.0], label='0 deg y.a.s. - BIAS + 3 STD', ls='-', color='black')

    line_3, = ax.plot(bin_centers, results_bias[2.0], label='2 deg y.a.s. - BIAS', ls=':', color='red')
    line_4, = ax.plot(bin_centers, results_3_std[2.0], label='2 deg y.a.s. - BIAS + 3 STD', ls='-', color='red')

    crit_line, = ax.plot(bin_centers, [crit_error_val] * len(bin_centers), label='deviation threshold', color='blue')

    ax.set_xlabel('range [m]')
    if 'pos' in deviation_column:
        ax.set_ylabel('deviation [m]')
    if 'vel' in deviation_column:
        ax.set_ylabel('deviation [m/s]')
    ax.set_title(title)
    ax.xaxis.set_ticks(np.arange(5, 65, 5))
    plt.grid()
    plt.legend(handles=[line_1, line_2, line_3, line_4, crit_line])
    file_name = f"{title.split()[-1]}_{deviation_column}_range_fun.png"
    save_path = os.path.join(SAVE_FIG_FOLDER, file_name)
    fig.savefig(save_path)
    return out_ref_data_filtered

