import numpy as np
import pyqtgraph as pg

from aspe.gui.drawers.DrawerErrCov import DrawerErrCov


class DrawerErrCovPosition(DrawerErrCov):
    def __init__(self,
                 parent,
                 name: str,
                 plot_item: pg.PlotItem,
                 color: str,
                 line_style: str,
                 line_width: float,
                 extracted_data_set):
        super().__init__(parent, name, plot_item, color, line_style, line_width, extracted_data_set)

    def calculate_points(self):
        ellipse_points = self.calc_cov_ellipse_points(
            self._change_series_to_numpy_representation(self.data_df.loc[:, "var_x"]),
            self._change_series_to_numpy_representation(self.data_df.loc[:, "var_y"]),
            self._change_series_to_numpy_representation(self.data_df.loc[:, "cov_xy"]),
            self.line_points, self.confidence_interval)
        ellipse_points = self.move_ellipse_to_center(
            ellipse_points,
            self._change_series_to_numpy_representation(self.data_df.loc[:, "position_x"]),
            self._change_series_to_numpy_representation(self.data_df.loc[:, "position_y"]))
        ellipse_x, ellipse_y = self.ellipse_data_process_to_display(ellipse_points)

        return ellipse_x, ellipse_y

    def move_ellipse_to_center(self, ellipse_points, center_x, center_y):
        ellipse_points[:, 0, :] = ellipse_points[:, 0, :] + np.tile(center_x, (self.line_points, 1)).T
        ellipse_points[:, 1, :] = ellipse_points[:, 1, :] + np.tile(center_y, (self.line_points, 1)).T

        return ellipse_points

    @staticmethod
    def cov_data_indexes():
        return [0, 1, 0, 0, 2, 0]
