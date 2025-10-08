import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns


class Plotter:
    def __init__(self, config):
        self.config = config
        plt.style.use(self.config['plot_style'])
        self.figures_save_path = self.config['figures_save_path']
        self.size = self.config['default_plot_size']

    def box_xy(self, df, x_column, y_column, title, **kwargs):
        if 'figsize' in kwargs:
            fig_size = kwargs['figsize']
        else:
            fig_size = self.size
        fig, axes = plt.subplots(figsize=fig_size)
        sns.boxplot(ax=axes, x=df.loc[:, x_column], y=df.loc[:, y_column], orient="h", whis=1.5)
        self.set_xy_axes_props(axes, **kwargs)
        axes.grid()
        axes.legend(axes.artists, list(df.loc[:, y_column].unique()))
        axes.get_yaxis().set_ticks([])
        plt.title(title)
        plt.tight_layout()
        plt.savefig(self._make_path(title, False))
        plt.close()
        return self._make_path(title)

    def bar(self, df, title, markers=None, **kwargs):
        if 'figsize' in kwargs:
            fig_size = kwargs['figsize']
        else:
            fig_size = self.size
        if df.empty:
            ax = plt.plot()
        else:
            ax = df.plot(kind='bar', figsize=fig_size, title=title, grid=True, rot=0)
            if markers:
                handles, labels = ax.get_legend_handles_labels()
                for label, value in markers.items():
                    handle, = ax.plot([-1, 1], [value, value], color='black', linewidth=1.5, linestyle='--', label=label)
                    handles.append(handle)
                    labels.append(label)
                ax.legend(handles, labels)
            self.set_xy_axes_props(ax, **kwargs)
        plt.savefig(self._make_path(title, False), bbox_inches='tight')
        plt.close()
        return self._make_path(title)

    def histogram(self, df, title, **kwargs):
        if 'figsize' in kwargs:
            fig_size = kwargs['figsize']
        else:
            fig_size = self.config['default_plot_size']
        if 'bins' in kwargs:
            bin_num = kwargs['bins']
        else:
            bin_num = 40
        if isinstance(df, pd.DataFrame):
            soft_count = len(df.columns)
            alpha_val = 1. / soft_count
        else:
            alpha_val = 1
        ax = df.plot(kind='hist', figsize=fig_size, title=title, grid=True, histtype='bar', alpha=alpha_val,
                     bins=bin_num)
        self.set_xy_axes_props(ax, **kwargs)
        plt.tight_layout()
        plt.savefig(self._make_path(title, False))
        plt.close()
        return self._make_path(title)

    def cdf(self, df, quant_df, title, reference=None, **kwargs):
        if 'figsize' in kwargs:
            fig_size = kwargs['figsize']
        else:
            fig_size = self.size
        ax = df.plot(figsize=fig_size, title=title, grid=True)
        if reference is not None:
            ax.plot(reference.index, reference, color='black', linestyle=':', label='reference distribution')
        if not len(quant_df) == 0:
            self.get_quant_lines(quant_df, ax)
        if 'xlabel' in kwargs:
            ax.set_xlabel(kwargs['xlabel'])
        if 'xlim' in kwargs:
            if not(np.isnan(kwargs['xlim']).any() or np.isinf(kwargs['xlim']).any()):
            	ax.set_xlim(kwargs['xlim'])
        ax.set_ylabel('Quantile [-]')
        ax.set_ylim((0, 1))
        ax.set_yticks(np.arange(0, 1, 0.1))
        plt.legend(loc=4)
        plt.tight_layout()
        plt.savefig(self._make_path(title, False))
        plt.close()
        return self._make_path(title)

    def get_quant_lines(self, quant_df, ax):
        for idx, column in enumerate(quant_df):
            q95 = quant_df.iloc[1, idx]
            q99 = quant_df.iloc[2, idx]
            line_color = ax.lines[idx]._color
            ax.plot(np.array([q95, q95]), np.array([0, 1]), color=line_color, linewidth=1.5, linestyle='--', label='q95')
            ax.plot(np.array([q99, q99]), np.array([0, 1]), color=line_color, linewidth=2, linestyle='--', label='q99')

    def pdf(self, df, title, reference=None, **kwargs):
        if 'figsize' in kwargs:
            fig_size = kwargs['figsize']
        else:
            fig_size = self.size
        ax = df.plot(figsize=fig_size, title=title, grid=True)
        if reference is not None:
            ax.plot(reference.index, reference, color='black', linestyle=':', label='reference distribution')
        self.set_xy_axes_props(ax, **kwargs)
        ax.set_ylabel('Density [-]')
        plt.legend()
        plt.tight_layout()
        plt.savefig(self._make_path(title, False))
        plt.close()
        return self._make_path(title)

    def box_xy_hue(self, df, x_col, y_col, hue_col, title, **kwargs):
        if 'figsize' in kwargs:
            fig_size = kwargs['figsize']
        else:
            fig_size = self.size
        fig, axes = plt.subplots(figsize=fig_size)
        sns.boxplot(ax=axes, x=df.loc[:, x_col], y=df.loc[:, y_col], hue=df.loc[:, hue_col], whis=1)
        plt.grid(True)
        if 'markers' in kwargs:
            handles, labels = axes.get_legend_handles_labels()
            for label, value in kwargs['markers'].items():
                handle, = axes.plot([-1, 1], [value, value], color='black', linewidth=1.5, linestyle='--',
                                       label=label)
                handles.append(handle)
                labels.append(label)
            axes.legend(handles, labels)
        self.set_xy_axes_props(axes, **kwargs)
        plt.title(title)
        plt.savefig(self._make_path(title, False))
        plt.close()
        return self._make_path(title)

    def trace(self, df, title, **kwargs):
        if 'figsize' in kwargs:
            fig_size = kwargs['figsize']
        else:
            fig_size = self.config['default_plot_size']
        fig, axes = plt.subplots(nrows=2, ncols=2)
        titles = [['REF TP', 'REF FN'], ['EST TP', 'EST FP']]
        x = self.config['host_size'][0] / 2
        y = self.config['host_size'][1] / 2
        for i in range(0, 2):
            for j in range(0, 2):
                if not df[0].empty:
                    df[0].plot(kind='scatter', x='position_y', y='position_x', figsize=fig_size, grid=True,
                               color='blue', ax=axes[i, j], s=1, alpha=0.03)
                if not df[1].empty:
                    df[1].plot(kind='scatter', x='position_y', y='position_x', figsize=fig_size, grid=True,
                               color='black', ax=axes[i, j], s=1, alpha=0.03)
                if not df[2].empty:
                    df[2].plot(kind='scatter', x='position_y', y='position_x', figsize=fig_size, grid=True,
                               color='green', ax=axes[i, j], s=1, alpha=0.03)
                if not df[3].empty:
                    df[3].plot(kind='scatter', x='position_y', y='position_x', figsize=fig_size, grid=True,
                               color='red', ax=axes[i, j], s=1, alpha=0.03)
                axes[i, j].plot([-x, x, x, -x, -x], [-y, -y, y, y, -y], color='black')
                self.set_xy_axes_props(axes[i, j], **kwargs)
                axes[i, j].set_xlim(self.config['trace_plots_lims'])
                axes[i, j].set_ylim(self.config['trace_plots_lims'])
                axes[0, 0].legend(['Host'])
        if not df[0].empty:
            df[0].plot(kind='scatter', x='position_y', y='position_x', figsize=fig_size, grid=True,
                       color='blue', ax=axes[0, 0], s=1, sharex=axes[1, 0], sharey=axes[0, 1])
        if not df[1].empty:
            df[1].plot(kind='scatter', x='position_y', y='position_x', figsize=fig_size, grid=True,
                       color='black', ax=axes[0, 1], s=1, sharex=axes[1, 1])
        if not df[2].empty:
            df[2].plot(kind='scatter', x='position_y', y='position_x', figsize=fig_size, grid=True,
                       color='green', ax=axes[1, 0], s=1, sharey=axes[1, 1])
        if not df[3].empty:
            df[3].plot(kind='scatter', x='position_y', y='position_x', figsize=fig_size, grid=True,
                       color='red', ax=axes[1, 1], s=1)

        for i in range(0, 2):
            for j in range(0, 2):
                axes[i, j].set_title(titles[i][j])
                axes[i, j].set_xlabel('position VCS Y [m]')
                axes[i, j].set_ylabel('position VCS X [m]')

        plt.tight_layout()
        plt.suptitle(title, fontsize=16, y=1)
        plt.savefig(self._make_path(title, False))
        plt.close()
        return self._make_path(title)

    def simple_trace(self, df, title, **kwargs):
        if 'figsize' in kwargs:
            fig_size = kwargs['figsize']
        else:
            fig_size = self.config['default_plot_size']
        fig, axes = plt.subplots(figsize=fig_size)
        host_wid = self.config['host_size'][0] / 2
        host_len = self.config['host_size'][1] / 2
        df.plot(kind='scatter', x='position_y', y='position_x', figsize=fig_size, grid=True,
                color='blue', ax=axes, s=1, alpha=0.3)
        axes.plot([-host_wid, host_wid, host_wid, -host_wid, -host_wid],
                  [-host_len, -host_len, host_len, host_len, -host_len], color='black')
        self.set_xy_axes_props(axes, **kwargs)
        axes.set_xlim(self.config['trace_plots_lims'])
        axes.set_ylim(self.config['trace_plots_lims'])
        axes.set_title(title)
        axes.set_xlabel('position VCS Y [m]')
        axes.set_ylabel('position VCS X [m]')
        axes.set_aspect('equal', adjustable='box')
        plt.tight_layout()
        plt.savefig(self._make_path(title, False))
        plt.close()
        return self._make_path(title)

    def _make_path(self, title, suffix=True):
        if suffix:
            return self.figures_save_path + '\\' + title.replace(' ', '_').replace('\n', '_') + '.png'
        else:
            return self.figures_save_path + '\\' + title.replace(' ', '_').replace('\n', '_')

    def set_xy_axes_props(self, ax, **kwargs):
        if 'xlabel' in kwargs:
            ax.set_xlabel(kwargs['xlabel'])
        if 'ylabel' in kwargs:
            ax.set_ylabel(kwargs['ylabel'])
        if 'xlim' in kwargs:
            ax.set_xlim(kwargs['xlim'])
        if 'ylim' in kwargs:
            ax.set_ylim(kwargs['ylim'])
        if 'yscale' in kwargs:
            ax.set_yscale(kwargs['yscale'])
        if 'legend_loc' in kwargs:
            ax.legend(loc=kwargs['legend_loc'])
        if 'remove_legend' in kwargs:
            if kwargs['remove_legend']:
                ax.get_legend().remove()
