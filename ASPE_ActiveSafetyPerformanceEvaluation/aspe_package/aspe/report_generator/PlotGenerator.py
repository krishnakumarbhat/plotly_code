import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns


class PlotGenerator:
    def __init__(self, sw_version_column, subset_column, hue_order, figsize=(12, 6), dodge=0.6, whiskers=1.5):
        self.sw_version_column = sw_version_column
        self.subset_column = subset_column
        self.fig_size = figsize
        self.dodge = dodge
        self.whiskers = whiskers
        self.hue_order = hue_order

    def box_plot(self, pairs_features_df, y_column, threshold=None):
        fig, ax = plt.subplots(figsize=self.fig_size)
        sns.boxplot(ax=ax, data=pairs_features_df, x=self.sw_version_column, y=y_column, hue=self.subset_column, whis=self.whiskers, hue_order=self.hue_order)
        plt.grid()
        fig.tight_layout()
        if threshold:
            ax.axhline(threshold, label='threshold', c='r', ls='--', lw=2)
        return fig

    def bar_plot_with_estimator(self, pairs_features_df, y_column, ylim=None, threshold=None, estimator=None):
        fig, ax = plt.subplots(figsize=self.fig_size)
        sns.barplot(ax=ax, data=pairs_features_df, x=self.sw_version_column, y=y_column, hue=self.subset_column, ci=None, estimator=estimator, hue_order=self.hue_order)
        plt.grid()
        fig.tight_layout()
        if ylim:
            ax.set_ylim(ylim)
        if threshold:
            ax.axhline(threshold, label='threshold', c='r', ls='--', lw=2)
        return fig

    def bar_plot(self, kpi_df, y_column, ylim=None, threshold=None):
        f, ax = plt.subplots(figsize=self.fig_size)
        sns.barplot(ax=ax, data=kpi_df, x=self.sw_version_column, y=y_column, hue=self.subset_column, dodge=self.dodge, hue_order=self.hue_order)
        ax.grid()
        if ylim:
            ax.set_ylim(ylim)
        if threshold:
            ax.axhline(threshold, label='threshold', c='r', ls='--', lw=2)
        return f


if __name__ == "__main__":
    example_data = r'C:\F360\for_data_frames_generations.xls'
    df_deviation = pd.read_excel(example_data, sheet_name='deviation')
    df_data = pd.read_excel(example_data, sheet_name='TPR')
    plotter = PlotGenerator(df_deviation,df_data, 'case', 'sw_version')
    a = plotter.box_plot('dev_x')
    b = plotter.bar_plot('TPR')
    plt.show()
