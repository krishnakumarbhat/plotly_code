from typing import List, Tuple, Union

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

from aspe.utilities.nexus50k_events_finder.detections_analysis.size_error_analysis.PlotConfig import PlotConfig

COLOR_PALETTE = [
    '#1f77b4',
    '#ff7f0e',
    '#5FD35F',
    '#d62728',
    '#9467bd',
    '#bcbd22',
    '#7f7f7f',
    '#17becf',
    '#8c564b',
    '#008000',
    '#800000',
    '#FF00FF',
    '#FFFF00',
    '#000000',
]


class SizeErrorPlotter:
    def __init__(self, plot_config: PlotConfig):
        self.plot_config = plot_config

        self.f_width, self.ax_width = plt.subplots(nrows=2, sharex=False, figsize=(13, 7), dpi=100)
        self.f_length, self.ax_length = plt.subplots(nrows=2, sharex=False, figsize=(13, 7), dpi=100)
        self.f_pos_x, self.ax_pos_x = plt.subplots(nrows=2, sharex=False, figsize=(13, 7), dpi=100)
        self.f_pos_y, self.ax_pos_y = plt.subplots(nrows=2, sharex=False, figsize=(13, 7), dpi=100)
        self.f_iou, self.ax_iou = plt.subplots(nrows=2, sharex=False, figsize=(13, 7), dpi=100)

        self.ax_width[0].set_title(f"Width error analysis - {plot_config.event_name}")
        self.ax_length[0].set_title(f"Length error analysis - {plot_config.event_name}")
        self.ax_pos_x[0].set_title(f"Position x error analysis - {plot_config.event_name}")
        self.ax_pos_y[0].set_title(f"Position y error analysis - {plot_config.event_name}")
        self.ax_iou[0].set_title(f"IoU analysis - {plot_config.event_name}")

        self.bin_column = self.plot_config.y_axis_value
        self.data_counter = 0

    def show(self):
        """Show plot"""
        plt.show()

    def legend(self):
        """Show legend."""
        for ax in [self.ax_width, self.ax_length, self.ax_pos_x, self.ax_pos_y, self.ax_iou]:
            ax[0].legend()
            ax[1].legend()

        for f in [self.f_width, self.f_length, self.f_pos_x, self.f_pos_y, self.f_iou]:
            f.tight_layout()

    def plot(self, df: pd.DataFrame, plots: list, resim_suffix: str, bin_width: float = 5):
        if 'error_width' in df.columns:
            self.plot_binned_data(df, plots, bin_width, 'error_width', self.ax_width, resim_suffix)
        if 'error_length' in df.columns:
            self.plot_binned_data(df, plots, bin_width, 'error_length', self.ax_length, resim_suffix)
        if 'error_position_x' in df.columns:
            self.plot_binned_data(df, plots, bin_width, 'error_position_x', self.ax_pos_x, resim_suffix)
        if 'error_position_y' in df.columns:
            self.plot_binned_data(df, plots, bin_width, 'error_position_y', self.ax_pos_y, resim_suffix)
        if 'iou' in df.columns:
            self.plot_binned_data(df, plots, bin_width, 'iou', self.ax_iou, resim_suffix)

        for axes in [self.ax_width, self.ax_length, self.ax_pos_x, self.ax_pos_y, self.ax_iou]:
            self.plot_samples_count(axes, resim_suffix)

        self.data_counter += 1

    def plot_binned_data(self, df: pd.DataFrame, plots: Union[List, Tuple], bin_width: float, y_column: str,
                         plot_ax: List[plt.Axes], resim_suffix: str):
        srtd = df.loc[:, [self.bin_column, y_column]].dropna().sort_values(by=self.bin_column)

        bins_borders, self.bins_centers = self.get_bins(self.bin_column, bin_width, srtd)

        self.bins_out = {item.name: [] for item in plots}

        self.bins_out['samples_count'] = []

        for left, right in bins_borders:
            bin_df = srtd.query(f'{left} < {self.bin_column} & {self.bin_column} < {right}')
            bin_y_vals = bin_df.loc[:, y_column].to_numpy()

            for item in plots:
                self.bins_out[item.name].append(item.compute(bin_y_vals))

            self.bins_out['samples_count'].append(len(bin_df))

        for item in plots:
            plot_ax[0].plot(self.bins_centers, self.bins_out[item.name],
                            label=f"{item.name} - {resim_suffix}",
                            linestyle=item.linestyle,
                            c=COLOR_PALETTE[self.data_counter],
                            alpha=item.alpha)

        plot_ax[0].set_xlabel(self.bin_column + f' [{self.plot_config.y_axis_units}]')
        plot_ax[1].set_xlabel(self.bin_column + f' [{self.plot_config.y_axis_units}]')
        plot_ax[0].set_ylabel(y_column + f' [{self.plot_config.y_axis_units}]')

    def plot_samples_count(self, plot_ax, resim_suffix: str):
        width = ((max(self.bins_centers) - min(self.bins_centers)) / len(self.bins_centers))
        div_result = self.plot_config.number_of_resims / 2

        if self.plot_config.number_of_resims % 2:
            bins_pos = (np.arange(-np.floor(div_result), np.floor(div_result) + 1,
                                  1)) / self.plot_config.number_of_resims
        else:
            bins_pos = np.arange(-self.plot_config.number_of_resims + 1, self.plot_config.number_of_resims, 2) / (
                    self.plot_config.number_of_resims * 2)

        plot_ax[1].bar(x=self.bins_centers + width * bins_pos[self.data_counter],
                       height=self.bins_out['samples_count'],
                       width=width / self.plot_config.number_of_resims,
                       label=f"samples_count_{resim_suffix}",
                       alpha=0.5)

        plot_ax[1].set_ylabel('samples_count')

    def grid(self):
        for axes in [self.ax_width, self.ax_length, self.ax_pos_x, self.ax_pos_y, self.ax_iou]:
            for ax in axes:
                ax.grid()

    def get_bins(self, bin_column: str, bin_width: float, srtd: pd.DataFrame):
        bin_col_values = srtd.loc[:, bin_column].to_numpy()
        bins_segments = np.arange(start=bin_col_values[0], stop=bin_col_values[-1] + bin_width, step=bin_width)
        bins_borders = list(zip(bins_segments, bins_segments[1:]))
        bins_centers = np.array([(lv + rv) / 2 for lv, rv in bins_borders])
        return bins_borders, bins_centers
