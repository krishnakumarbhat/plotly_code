import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os


SAVE_FIG_FOLDER = r'P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\Analysis\analysis_plots'
DEFAULT_PLOT_SIZE = (10, 7.5)


def get_bin_class_plots_in_range_fun(ref_df: pd.DataFrame, est_df: pd.DataFrame):
    results_tpr = {}
    results_ppv = {}
    results_split_ratio = {}
    range_bins = [(low * 5.0, (low + 1) * 5.0) for low in range(1, 12)]
    bin_centers = [(bin[0] + bin[1])/2 for bin in range_bins]
    for align_angle, ref_data in ref_df.groupby(by='align_angle'):
        est_data = est_df.loc[est_df.align_angle == align_angle, :]
        tpr_vec = []
        ppv_vec = []
        split_ratio_vec = []
        for bin in range_bins:
            range_mask_ref = np.logical_and(bin[0] < ref_data.range.values, ref_data.range.values < bin[1])
            range_mask_est = np.logical_and(bin[0] < est_data.range.values, est_data.range.values < bin[1])

            ref_filtered = ref_data.loc[range_mask_ref, :]
            est_filtered = est_data.loc[range_mask_est, :]

            tp_count = np.sum(ref_filtered.binary_classification == 'ref_tp')
            fn_count = np.sum(ref_filtered.binary_classification == 'fn')
            fp_count = np.sum(est_filtered.binary_classification == 'fp')

            tpr = 100 * tp_count / (tp_count + fn_count)
            ppv = 100 * tp_count / (tp_count + fp_count)
            split_ratio = 100 * fp_count / tp_count

            tpr_vec.append(tpr)
            ppv_vec.append(ppv)
            split_ratio_vec.append(split_ratio)

        results_tpr[align_angle] = tpr_vec
        results_ppv[align_angle] = ppv_vec
        results_split_ratio[align_angle] = split_ratio_vec
    fig_tpr, ax = plt.subplots(figsize=DEFAULT_PLOT_SIZE)
    line_1, = ax.plot(bin_centers, results_tpr[0.0], label='0 deg s.a. - TPR', ls='-', lw=3, color='black')
    line_2, = ax.plot(bin_centers, results_tpr[2.0], label='2 deg s.a. - TPR', ls='-', lw=3, color='red')
    desired_line_tpr, = ax.plot(bin_centers, [100] * len(bin_centers), label='desired value', lw=3, color='blue')

    plt.legend(handles=[line_1, line_2, desired_line_tpr])
    ax.set_xlabel('range [m]')
    ax.set_ylabel('KPI value [%]')
    ax.set_title('TPR in range function')
    ax.set_ylim([70, 102])
    ax.xaxis.set_ticks(np.arange(5, 65, 5))
    plt.grid()

    fig_ppv, ax_2 = plt.subplots(figsize=DEFAULT_PLOT_SIZE)
    line_3, = ax_2.plot(bin_centers, results_ppv[0.0], label='0 deg y.a.s. - PPV', ls='-', lw=3, color='black')
    line_4, = ax_2.plot(bin_centers, results_ppv[2.0], label='2 deg y.a.s. - PPV', ls='-', lw=3, color='red')
    desired_line_ppv, = ax_2.plot(bin_centers, [99.9] * len(bin_centers), label='desired value', lw=3, color='blue')
    plt.legend(handles=[line_3, line_4, desired_line_ppv])

    ax_2.set_xlabel('range [m]')
    ax_2.set_ylabel('KPI value [%]')
    ax_2.set_title('PPV in range function')
    ax_2.set_ylim([70, 102])
    ax_2.xaxis.set_ticks(np.arange(5, 65, 5))
    plt.grid()

    fig_split, ax_3 = plt.subplots(figsize=DEFAULT_PLOT_SIZE)
    line_5, = ax_3.plot(bin_centers, results_split_ratio[0.0], label='0 deg y.a.s. - Split Ratio', ls='-', lw=3, color='black')
    line_6, = ax_3.plot(bin_centers, results_split_ratio[2.0], label='2 deg y.a.s. - Split Ratio', ls='-', lw=3, color='red')
    desired_line_splits, = ax_3.plot(bin_centers, [0.1] * len(bin_centers), label='desired value', lw=3, color='blue')
    plt.legend(handles=[line_5, line_6, desired_line_splits])

    ax_3.set_xlabel('range [m]')
    ax_3.set_ylabel('KPI value [%]')
    ax_3.set_title('Split Ratio in range function')
    ax_3.set_ylim([0, 50])
    ax_3.xaxis.set_ticks(np.arange(5, 65, 5))
    plt.grid()

    file_name_ppv = f"RCTA_PPV_in_range_fun.png"
    file_name_tpr = f"RCTA_TPR_in_range_fun.png"
    file_name_splits = f"RCTA_Splits_in_range_fun.png"

    save_path_ppv = os.path.join(SAVE_FIG_FOLDER, file_name_ppv)
    save_path_tpr = os.path.join(SAVE_FIG_FOLDER, file_name_tpr)
    save_path_splits = os.path.join(SAVE_FIG_FOLDER, file_name_splits)

    fig_tpr.savefig(save_path_tpr)
    fig_ppv.savefig(save_path_ppv)
    fig_split.savefig(save_path_splits)


def get_far_ranges_bin_class_plots(ref_df: pd.DataFrame, est_df, range_low, range_high):
    far_ranges_mask_ref = np.logical_and(range_low < ref_df.range.values, ref_df.range.values < range_high)
    far_ranges_mask_est = np.logical_and(range_low < est_df.range.values, est_df.range.values < range_high)
    ref_data_far_range = ref_df.loc[far_ranges_mask_ref, :]
    est_data_far_range = est_df.loc[far_ranges_mask_est, :]
    tpr_vec = []
    ppv_vec = []
    angles_vec = []
    split_ratio_vec = []
    for align_angle, ref_data in ref_data_far_range.groupby(by='align_angle'):
        est_data = est_data_far_range.loc[est_data_far_range.align_angle == align_angle, :]

        tp_count = np.sum(ref_data.binary_classification == 'ref_tp')
        fn_count = np.sum(ref_data.binary_classification == 'fn')
        fp_count = np.sum(est_data.binary_classification == 'fp')

        tpr = 100 * tp_count / (tp_count + fn_count)
        ppv = 100 * tp_count / (tp_count + fp_count)
        split_ratio = 100 * fp_count / tp_count

        tpr_vec.append(tpr)
        ppv_vec.append(ppv)
        split_ratio_vec.append(split_ratio)
        angles_vec.append(align_angle)
    fig, ax_1 = plt.subplots(figsize=DEFAULT_PLOT_SIZE)
    ax_1.set_xlabel('yaw angle shift [deg]')
    ax_1.set_ylabel('KPI value [%]')
    tpr_line, = ax_1.plot(angles_vec, tpr_vec, label='TPR', lw=3, color='red')
    desired_line_tpr, = ax_1.plot(angles_vec, [100] * len(angles_vec), label='desired value', lw=3, color='blue')
    plt.legend(handles=[tpr_line, desired_line_tpr])

    ax_1.set_title('TPR')
    ax_1.set_ylim([70, 102])
    plt.grid()
    file_name = f'RCTA_TPR_range_{range_low}_{range_high}'
    fig.savefig(os.path.join(SAVE_FIG_FOLDER, file_name))

    fig, ax_2 = plt.subplots(figsize=DEFAULT_PLOT_SIZE)
    ppv_line, = ax_2.plot(angles_vec, ppv_vec, color='red', label='PPV', lw=3)
    desired_line_ppv, = ax_2.plot(angles_vec, [99.75] * len(angles_vec), label='desired value', lw=3, color='blue')
    plt.legend(handles=[ppv_line, desired_line_ppv])
    ax_2.set_xlabel('yaw angle shift [deg]')
    ax_2.set_ylabel('KPI value [%]')
    ax_2.set_title('PPV')
    ax_2.set_ylim([70, 102])
    plt.grid()
    file_name = f'RCTA_PPV_range_{range_low}_{range_high}'
    fig.savefig(os.path.join(SAVE_FIG_FOLDER, file_name))

    fig, ax_3 = plt.subplots(figsize=DEFAULT_PLOT_SIZE)
    split_ratio_line, = ax_3.plot(angles_vec, split_ratio_vec, color='red', label='Split Ratio', lw=3)
    desired_line_splits, = ax_3.plot(angles_vec, [0.1] * len(angles_vec), label='desired value', lw=3, color='blue')
    plt.legend(handles=[split_ratio_line, desired_line_splits])
    ax_3.set_xlabel('yaw angle shift [deg]')
    ax_3.set_ylabel('KPI value [%]')
    ax_3.set_title('Split Ratio')
    ax_3.set_ylim([0, 50])
    plt.grid()
    file_name = f'RCTA_Splits_range_{range_low}_{range_high}'
    fig.savefig(os.path.join(SAVE_FIG_FOLDER, file_name))