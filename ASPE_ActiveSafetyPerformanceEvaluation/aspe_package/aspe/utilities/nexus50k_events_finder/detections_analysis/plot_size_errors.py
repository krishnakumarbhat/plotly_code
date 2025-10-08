import matplotlib.colors as colors
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy.stats import binned_statistic_2d

from aspe.utilities.MathFunctions import normalize_angle_vector
from aspe.utilities.nexus50k_events_finder.detections_analysis.utils import (
    calc_aspect_angle,
    calc_range,
    calc_speed,
    filter_only_cars,
    filter_only_moving,
    filter_only_same_dir,
)


def plot_binned_data_2d(df: pd.DataFrame, bin_column_x: str, bin_column_y: str, vals_column: str, bins=100, title=''):
    df = df.loc[:, [bin_column_x, bin_column_y, vals_column]].dropna()
    x = df.loc[:, bin_column_x].to_numpy()
    y = df.loc[:, bin_column_y].to_numpy()
    vals = df.loc[:, vals_column].to_numpy()

    mean_bins, xedges, yedges, binnums = binned_statistic_2d(x, y, vals, bins=bins, statistic='mean')
    std_bins, _, _, _ = binned_statistic_2d(x, y, vals, bins=bins, statistic='std')
    count_bins, _, _, _ = binned_statistic_2d(x, y, vals, bins=bins, statistic='count')

    low_count_bins = count_bins < 50
    mean_bins[low_count_bins] = np.nan
    std_bins[low_count_bins] = np.nan

    XX, YY = np.meshgrid(xedges, yedges)

    f, ax = plt.subplots(nrows=3, sharex=True)

    cm = 'jet'
    pc_1 = ax[0].pcolor(XX, YY, mean_bins.T, cmap=cm)
    pc_2 = ax[1].pcolor(XX, YY, std_bins.T, cmap=cm)
    pc_3 = ax[2].pcolor(XX, YY, count_bins.T, cmap=cm, norm=colors.LogNorm(vmin=0.1, vmax=count_bins.max()))

    for a in ax:
        a.grid()
        a.set_ylabel(bin_column_y)

    ax[-1].set_xlabel(bin_column_x)

    ax[0].set_title(f'mean of {vals_column}')
    ax[1].set_title(f'std of {vals_column}')
    ax[2].set_title(f'samples count')
    ax[2].set_title(f'samples count')
    f.suptitle(title)

    f.colorbar(pc_1, ax=ax[0])
    f.colorbar(pc_2, ax=ax[1])
    f.colorbar(pc_3, ax=ax[2])

    x_bin_centers = np.array([(n + m) / 2 for n, m in zip(xedges, xedges[1:])])
    y_bin_centers = np.array([(n + m) / 2 for n, m in zip(yedges, yedges[1:])])
    XX_center, YY_center = np.meshgrid(x_bin_centers, y_bin_centers)

    x_bin_centers_flat = XX_center.reshape(-1)
    y_bin_centers_flat = YY_center.reshape(-1)
    mean_bins_flat = mean_bins.T.reshape(-1)
    std_bins_flat = std_bins.T.reshape(-1)
    samples_count_flat = count_bins.T.reshape(-1)

    bins_data = pd.DataFrame({
        f'{bin_column_x}_bin_center_x': x_bin_centers_flat,
        f'{bin_column_y}_bin_center_y': y_bin_centers_flat,
        f'{vals_column}_bin_mean': mean_bins_flat,
        f'{vals_column}_bin_std': std_bins_flat,
        'samples_in_bin': samples_count_flat,
    })
    bins_data.dropna(inplace=True)

    return f, ax, bins_data


def plot_binned_data_1d(df: pd.DataFrame, bin_column: str, bin_width: float, y_column: str):
    srtd = df.loc[:, [bin_column, y_column]].dropna().sort_values(by=bin_column)

    bin_col_vals = srtd.loc[:, bin_column].to_numpy()
    bins_segments = np.arange(start=bin_col_vals[0], stop=bin_col_vals[-1] + bin_width, step=bin_width)

    bins_borders = list(zip(bins_segments, bins_segments[1:]))
    bins_centers = np.array([(lv + rv) / 2 for lv, rv in bins_borders])

    bins_out = {
        'center': [],
        'mean': [],
        'std': [],
        'median': [],
        'q05': [],
        'q95': [],
        'samples_count': [],
    }

    for left, right in bins_borders:
        bin_df = srtd.query(f'{left} < {bin_column} & {bin_column} < {right}')
        bin_y_vals = bin_df.loc[:, y_column].to_numpy()
        if len(bin_y_vals):
            bins_out['center'].append(np.mean([left, right]))
            bins_out['mean'].append(np.mean(bin_y_vals))
            bins_out['median'].append(np.median(bin_y_vals))
            bins_out['std'].append(np.std(bin_y_vals))
            bins_out['q95'].append(np.quantile(bin_y_vals, 0.95))
            bins_out['q05'].append(np.quantile(bin_y_vals, 0.05))
            bins_out['samples_count'].append(len(bin_df))

    f, ax = plt.subplots(nrows=2, sharex=True)

    max_y_val = max(abs(np.nanquantile(df.loc[:, y_column].to_numpy(), 0.01)),
                    abs(np.nanquantile(df.loc[:, y_column].to_numpy(), 0.99)))

    ax[0].plot(bins_out['center'], bins_out['median'], label='median')
    ax[0].plot(bins_out['center'], bins_out['q05'], label='q05', ls='--')
    ax[0].plot(bins_out['center'], bins_out['q95'], label='q95', ls='--')
    ax[1].plot(bins_out['center'], bins_out['samples_count'])

    ax[-1].set_xlabel(bin_column)
    ax[0].set_ylabel(y_column)
    ax[1].set_ylabel('samples_count')
    ax[0].set_ylim([-max_y_val, max_y_val])

    for a in ax:
        a.grid()
    ax[0].legend()
    return f, ax


def load_overtaking_data(auto_gt_p: str):
    auto_gt = pd.read_csv(auto_gt_p, index_col=0)

    calc_aspect_angle(auto_gt)
    calc_speed(auto_gt)
    calc_range(auto_gt)
    filter_only_same_dir(auto_gt)
    auto_gt = filter_only_cars(auto_gt)
    auto_gt = filter_only_moving(auto_gt)
    return auto_gt


def load_cta_data(auto_gt_p: str):
    auto_gt = pd.read_csv(auto_gt_p, index_col=0)

    calc_aspect_angle(auto_gt)
    calc_speed(auto_gt)
    calc_range(auto_gt)
    auto_gt = filter_only_cars(auto_gt)
    auto_gt = filter_only_moving(auto_gt)
    return auto_gt


def merge_dataframes(df_list: list):
    bins_data = pd.concat(df_list, axis=1)
    bins_data = bins_data.loc[:, ~bins_data.columns.duplicated()]
    return bins_data


if __name__ == '__main__':
    overtaking_data = load_overtaking_data(r"E:\logfiles\nexus_size_estimation_analysis\overtaking_auto_gt_TEMP.csv")
    cta_data = load_cta_data(r"E:\logfiles\nexus_size_estimation_analysis\cta_auto_gt.csv")
    bin_size = (40, 20)
    x_col, y_col = 'range', 'aspect_angle'

    # ONLY OVERTAKING --------------------------------------------------------------------------------------------------
    # f, ax = plot_binned_data_1d(overtaking_data.query('range<50'), 'range', 2.0, 'pseudo_err_len')
    f, ax, ps_err_len_bins = plot_binned_data_2d(overtaking_data, x_col, y_col, 'pseudo_err_len', bins=bin_size,
                                                 title='Overtaking events')
    f, ax, ps_err_wid_bins = plot_binned_data_2d(overtaking_data, x_col, y_col, 'pseudo_err_wid', bins=bin_size,
                                                 title='Overtaking events')

    overtaking_pseudo_errors_bins = merge_dataframes([ps_err_len_bins, ps_err_wid_bins])
    overtaking_pseudo_errors_bins.to_csv(
        r"E:\logfiles\nexus_size_estimation_analysis\overtaking_pseudo_error_binned_data.csv")

    # ONLY CTA ---------------------------------------------------------------------------------------------------------
    # f, ax = plot_binned_data_1d(cta_data.query('range<50'), 'range', 2.0, 'pseudo_err_len')
    f, ax, ps_err_len_bins = plot_binned_data_2d(cta_data, x_col, y_col, 'pseudo_err_len', bins=bin_size,
                                                 title='Cross-traffic events')
    f, ax, ps_err_wid_bins = plot_binned_data_2d(cta_data, x_col, y_col, 'pseudo_err_wid', bins=bin_size,
                                                 title='Cross-traffic events')

    cta_pseudo_errors_bins = merge_dataframes([ps_err_len_bins, ps_err_wid_bins])
    cta_pseudo_errors_bins.to_csv(r"E:\logfiles\nexus_size_estimation_analysis\cta_pseudo_error_binned_data.csv")

    # OVERTAKING + CTA -------------------------------------------------------------------------------------------------
    overtaking_cta_data = overtaking_data.append(cta_data).reset_index(drop=True)
    f, ax, ps_err_len_bins = plot_binned_data_2d(overtaking_cta_data, x_col, y_col, 'pseudo_err_len', bins=bin_size,
                                                 title='Overtaking + cross-traffic events')
    f, ax, ps_err_wid_bins = plot_binned_data_2d(overtaking_cta_data, x_col, y_col, 'pseudo_err_wid', bins=bin_size,
                                                 title='Overtaking + cross-traffic events')

    overtaking_cta_pseudo_errors_bins = merge_dataframes([ps_err_len_bins, ps_err_wid_bins])
    overtaking_cta_pseudo_errors_bins.to_csv(
        r"E:\logfiles\nexus_size_estimation_analysis\overtaking_cta_pseudo_error_binned_data.csv")
