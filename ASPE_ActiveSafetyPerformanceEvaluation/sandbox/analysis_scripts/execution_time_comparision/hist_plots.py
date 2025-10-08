import matplotlib.pyplot as plt
import pandas as pd

from extract_mudp import local_mudp_load


def plot_hist(path, pre_name):
    df_min, df_max, df_mean = local_mudp_load(path)

    def plt_n_save(name, df):
        plt.subplot(1, 2, 1)

        plt.ylabel('logs quantity')

        plt.suptitle(name + '_cl_gr_and_overall')
        plt.xlabel('mean time [%]')
        plt.title('cluster_grouping')
        hist_clus_gr = df.cluster_grouping.hist()
        plt.subplot(1, 2, 2)
        plt.title('time_taken_core_tracker')
        hist_overall = df.time_taken_core_tracker.hist()

        plt.xlabel('mean time [%]')

        plt.savefig(name + '_cl_gr_and_overall.png')
        plt.close()

    plt_n_save(pre_name + '_mean', df_mean)
    plt_n_save(pre_name + '_min', df_min)
    plt_n_save(pre_name + '_max', df_max)


def plot_multi_hists(path1, path2):
    df_min1, df_max1, df_mean1 = local_mudp_load(path1)

    df_min2, df_max2, df_mean2 = local_mudp_load(path2)

    def plot(df1, df2, name):
        df_plot = pd.DataFrame(data=df1.time_taken_core_tracker.values,
                               columns=['orig'])
        df_plot['changes'] = df2.time_taken_core_tracker.values
        df_plot.plot.hist(bins=12, alpha=0.5)

        plt.xlabel('time [%]')
        plt.ylabel('logs quantity')

        plt.suptitle(name + 'overall_time')
        plt.savefig(name + '_comparision_cl_gr_and_overall.png')
        plt.close()

    plot(df_min1, df_min2, 'min')
    plot(df_max1, df_max2, 'max')
    plot(df_mean1, df_mean2, '')


if __name__ == '__main__':
    path = r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\aggr_norm_diffs_dataframes_median.pickle"
    plot_hist(path, 'norm_diff')

    path = r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\aggr_raw_diffs_dataframes_median.pickle"
    plot_hist(path, 'raw_diff')

    path = r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\aggr_tag1_raw_params_dataframes_median.pickle"
    plot_hist(path, 'tag1_raw_params')

    path = r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\aggr_tag2_raw_params_dataframes_median.pickle"
    plot_hist(path, 'tag2_raw_params')

    path1 = r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\aggr_tag1_raw_params_dataframes_median.pickle"
    path2 = r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\aggr_tag2_raw_params_dataframes_median.pickle"
    plot_multi_hists(path1, path2)
