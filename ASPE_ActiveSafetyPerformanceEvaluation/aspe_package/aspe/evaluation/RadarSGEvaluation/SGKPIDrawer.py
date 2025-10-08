import matplotlib.pyplot as plt
import numpy as np
from matplotlib.lines import Line2D
from matplotlib.widgets import Button

from aspe.utilities.SupportingFunctions import load_from_pkl

fig, ax = plt.subplots()
fig.subplots_adjust(bottom=0.2)

custom_lines = {
    'evaluated segments': {'color': 'blue', 'lw': 3},
    'associated segments': {'color': 'orange', 'lw': 3},
    'association gates': {'color': 'black', 'linestyle': '--', 'alpha': 0.4, 'lw': 3},
    'TP samples': {'color': 'greenyellow', 'marker': 'o'},
    'FP samples': {'color': 'crimson', 'marker': 'o'},
}


def add_legend():
    legend_lines = [Line2D([0], [0], **value) for value in custom_lines.values()]
    ax.legend(legend_lines, custom_lines.keys())


def get_data(file_path):
    data = load_from_pkl(file_path)
    title = data[0]
    SG_estimated = data[1].signals
    SG_segments = data[2].signals

    SG_segments['coords_x'] = SG_segments.apply(lambda row: np.array([row['start_position_x'], row['end_position_x']]),
                                                axis=1)
    SG_segments['coords_y'] = SG_segments.apply(lambda row: np.array([row['start_position_y'], row['end_position_y']]),
                                                axis=1)

    mask = SG_estimated['classification'].values
    TP_df = SG_estimated[mask].copy()
    FP_df = SG_estimated[~mask].copy()
    return TP_df, FP_df, SG_segments, title


class KPIDrawer:
    def __init__(self, TP_estimated_df, FP_estimated_df, SG_segments_estimated, SG_segments_reference, title):
        self.TP_estimated_df = TP_estimated_df
        self.FP_estimated_df = FP_estimated_df
        self.SG_segments_estimated = SG_segments_estimated
        self.SG_segments_reference = SG_segments_reference
        self.title = title

    def draw(self, scan):
        ax.cla()
        ax.set_title(' '.join(["KPI's plot - ", self.title]))
        ax.set_xlabel("x-axis (m)")
        ax.set_ylabel("y-axis (m)")

        ax.grid(True)

        self.draw_segments(self.SG_segments_estimated[self.SG_segments_estimated['scan_index'] == scan],
                           **custom_lines['evaluated segments'])
        self.draw_segments(self.SG_segments_reference[self.SG_segments_reference['scan_index'] == scan],
                           **custom_lines['associated segments'])
        self.draw_samples(self.FP_estimated_df[self.FP_estimated_df['scan_index'] == scan],
                          **custom_lines['FP samples'])
        self.draw_samples(self.TP_estimated_df[self.TP_estimated_df['scan_index'] == scan],
                          **custom_lines['TP samples'])

        ax.axis('equal')
        fig.canvas.draw()
        add_legend()

    def draw_projection(self, scan):
        self.draw_projection_samples(self.TP_estimated_df[self.TP_estimated_df['scan_index'] == scan],
                                     **custom_lines['TP samples'])
        self.draw_projection_samples(self.FP_estimated_df[self.FP_estimated_df['scan_index'] == scan],
                                     **custom_lines['FP samples'])
        fig.canvas.draw()

    @staticmethod
    def draw_segments(SG_segments, **linestyle):
        if SG_segments.items() is not None:
            SG_segments.apply(lambda row: ax.plot(row['coords_x'], row['coords_y'], **linestyle), axis=1)

    @staticmethod
    def draw_samples(SG_estimated, **linestyle):
        if SG_estimated.items() is not None:
            ax.scatter(SG_estimated['sample_position_x'], SG_estimated['sample_position_y'], linewidths=4, **linestyle)

    @staticmethod
    def draw_projection_samples(SG_estimated, **linestyle):
        if SG_estimated.items() is not None:
            ax.scatter(SG_estimated['projection_sample_position_x'], SG_estimated['projection_sample_position_y'],
                       linewidths=4,
                       alpha=0.2, **linestyle)


class PlotButtons:
    def __init__(self, TP_estimated_df, FP_estimated_df, SG_segments_estimated, SG_segments_reference, title):
        self.current_position = 0
        self.drawer = KPIDrawer(TP_estimated_df, FP_estimated_df, SG_segments_estimated, SG_segments_reference, title)
        self.unique_scans = np.sort(self.drawer.SG_segments_estimated['scan_index'].unique())
        self._max = len(self.unique_scans) - 1

    def next(self, _):
        if self.current_position < self._max:
            self.current_position += 1
            self.drawer.draw(self.unique_scans[self.current_position])
            plt.title(f'current scan: {self.current_position}')
        else:
            plt.title('last scan!')

    def prev(self, _):
        if self.current_position > 0:
            self.current_position -= 1
            self.drawer.draw(self.unique_scans[self.current_position])
            plt.title(f'current scan: {self.current_position}')
        else:
            plt.title('first scan!')

    def projection(self, _):
        self.drawer.draw_projection(self.unique_scans[self.current_position])
        self.drawer.draw_projection(self.unique_scans[self.current_position])


if __name__ == '__main__':
    evaluated_log_path = r"estimation.pickle"
    associated_log_path = r"reference.pickle"

    TP_estimated_df, FP_estimated_df, SG_segments_estimated, eval_title = get_data(evaluated_log_path)
    _, _, SG_segments_reference, _ = get_data(associated_log_path)

    callback = PlotButtons(TP_estimated_df, FP_estimated_df, SG_segments_estimated, SG_segments_reference, eval_title)
    ax_projection = fig.add_axes([0.55, 0.05, 0.1, 0.075])
    b_projection = Button(ax_projection, 'Draw Projection')
    b_projection.on_clicked(callback.projection)

    ax_prev = fig.add_axes([0.7, 0.05, 0.1, 0.075])
    b_prev = Button(ax_prev, 'Previous')
    b_prev.on_clicked(callback.prev)

    ax_next = fig.add_axes([0.81, 0.05, 0.1, 0.075])
    b_next = Button(ax_next, 'Next')
    b_next.on_clicked(callback.next)
    plt.show()
