from aspe.extractors.API.mdf import extract_f360_bmw_mid_from_mf4
import numpy as np
from warnings import warn
import matplotlib.pyplot as plt
import os


def plot_timing_info(log_path: str, save_path=None):
    extr = extract_f360_bmw_mid_from_mf4(log_path, raw_signals=True, sw_version='A410', save_to_file=True)

    ptp_sigs = extr.ptp_data.signals
    obj_list_header_sigs = extr.objects_header.signals
    vigem_to_global_time_diff_median = extr.ptp_data.vigem_to_global_time_diff_median
    vigem_to_global_time_diff_spread_ms = extr.ptp_data.vigem_to_global_time_diff_spread * 1e3
    if vigem_to_global_time_diff_spread_ms > 1:
        warn('Spread of difference between vigem timestamp and global timestamp is above 1 [ms]')

    obj_list_header_sigs['vigem_to_global_time_diff_median'] = vigem_to_global_time_diff_median
    obj_list_header_sigs['publish_time_in_global_domain'] = obj_list_header_sigs['vigem_timestamp'] - vigem_to_global_time_diff_median
    obj_list_header_sigs['system_latency'] = obj_list_header_sigs['publish_time_in_global_domain'] - obj_list_header_sigs['timestamp']

    f, axes = plt.subplots(nrows=3, ncols=2, gridspec_kw={'width_ratios': [3, 2]}, figsize=(15, 9))
    vigem_to_global_time_diff_ms = (ptp_sigs.vigem_to_global_time_diff - vigem_to_global_time_diff_median) * 1e3
    vigem_to_global_time_diff_ms.plot(ax=axes[0, 0], label='observed')
    vigem_to_global_time_diff_ms.hist(ax=axes[0, 1])

    obj_list_ts_ms = obj_list_header_sigs.timestamp * 1e3
    obj_list_ts_diffs = obj_list_ts_ms.diff()
    obj_list_ts_diffs.plot(ax=axes[1, 0], label='observed')
    obj_list_ts_diffs.hist(ax=axes[1, 1])

    system_latency_ms = obj_list_header_sigs['system_latency'] * 1e3
    system_latency_ms.plot(ax=axes[2, 0], label='observed')
    system_latency_ms.hist(ax=axes[2, 1])

    axes[0, 0].set_title('Synchronization signals consistency: difference betwen PTP timestamp \n and vigem timestamp in relation to median difference')
    axes[0, 1].set_title('Synchronization signals consistency - histogram')
    axes[0, 0].set_xlabel('time instance [-]')
    axes[0, 0].set_ylabel('time diff [ms]')
    axes[0, 1].set_xlabel('time diff [ms]')
    axes[0, 1].set_ylabel('samples count [-]')

    axes[1, 0].set_title('Object list timestamp diff')
    axes[1, 1].set_title('Object list timestamp diff - histogram')
    axes[1, 0].set_xlabel('time instance [-]')
    axes[1, 0].set_ylabel('time diff [ms]')
    axes[1, 1].set_xlabel('time diff [ms]')
    axes[1, 1].set_ylabel('samples count [-]')

    axes[2, 0].set_title('Object list system latency')
    axes[2, 1].set_title('Object list system latency - histogram')
    axes[2, 0].set_xlabel('time instance [-]')
    axes[2, 0].set_ylabel('time [ms]')
    axes[2, 1].set_xlabel('time [ms]')
    axes[2, 1].set_ylabel('samples count [-]')

    x_beg, x_end = vigem_to_global_time_diff_ms.index[0], vigem_to_global_time_diff_ms.index[-1]
    axes[0, 0].plot([x_beg, x_end, np.nan, x_beg, x_end], [-0.1, -0.1, np.nan, 0.1, 0.1], '--', label='expectation')

    x_beg, x_end = obj_list_ts_diffs.index[0], obj_list_ts_diffs.index[-1]
    axes[1, 0].plot([x_beg, x_end], [50, 50], '--', label='expectation')

    x_beg, x_end = system_latency_ms.index[0], system_latency_ms.index[-1]
    axes[2, 0].plot([x_beg, x_end], [250, 250], '--', label='expectation')

    for ax in axes:
        ax[0].grid()
        ax[0].legend(loc=2)
    f.tight_layout()
    if save_path is not None:
        _, log_name = os.path.split(log_path)
        log_name, _ = os.path.splitext(log_name)
        f.savefig(save_path + f'{log_name}_timing_info.png')


if __name__ == '__main__':
    log_path = r'C:\LOGS\PATH\TO\MDF\FILE.mf4'
    save_path = None  # paste here directory path if you want to save plot to .png file
    plot_timing_info(log_path, save_path)