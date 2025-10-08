import pandas as pd
import matplotlib.pyplot as plt
import os
DEFAULT_PLOT_SIZE = (8, 8)

SAVEFIG_FOLDER = r'P:\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\Analysis\analysis_plots'


def plot_traces(ref_df: pd.DataFrame, title):
    ref_df = ref_df.loc[ref_df.align_angle == 0.0, :]
    range_mask = ref_df.range.values < 90
    ref_df = ref_df.loc[range_mask, :]
    fig, ax = plt.subplots()
    for log_idx, log_df in ref_df.groupby(by='log_idx'):
        ax.plot(log_df.position_vcsy, log_df.position_vcsx, color='blue', alpha=0.7)
    ax.plot([-1, -1, 1, 1, -1], [0, -4, -4, 0, 0], color='red')
    ax.set_aspect('equal', 'box')
    ax.set_xlabel('position VCS Y [m]')
    ax.set_ylabel('position VCS X [m]')
    ax.axis([-95, 95, -95, 95])
    ax.set_title(title)
    ax.grid()
    save_path = os.path.join(SAVEFIG_FOLDER, 'RCTA_trace_plots.png')
    fig.savefig(save_path)