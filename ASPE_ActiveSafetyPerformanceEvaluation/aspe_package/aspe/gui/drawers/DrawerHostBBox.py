import numpy as np
import pyqtgraph as pg

from aspe.gui.drawers.DrawerFilledPolygon import DrawerFilledPolygon
from aspe.utilities.MathFunctions import calc_position_in_bounding_box


class DrawerHostBBox(DrawerFilledPolygon):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str):
        super().__init__(parent, name, plot_item, color)
        self.graphic_object.mousePressEvent = self.on_click

    def on_click(self, mouse_click_event):
        self.parent.on_click(self, mouse_click_event)

    def set_data(self, data_df, pos_x_signature, pos_y_signature, length, width, ref_point_x=1.0, ref_point_y=0.5):
        pos_x = data_df.loc[:, pos_x_signature]
        pos_y = data_df.loc[:, pos_y_signature]
        center_x, center_y = \
            calc_position_in_bounding_box(pos_x, pos_y, length, width, 0.0, ref_point_x, ref_point_y, 0.5, 0.5)
        center_x = center_x.to_numpy()
        center_y = center_y.to_numpy()
        corner_x_RL, corner_y_RL = center_x - length/2, center_y - width/2
        corner_x_RR, corner_y_RR = center_x - length/2, center_y + width/2
        corner_x_FR, corner_y_FR = center_x + length/2, center_y + width/2
        corner_x_FL, corner_y_FL = center_x + length/2, center_y - width/2

        corners_x = np.vstack([corner_x_RL, corner_x_RR, corner_x_FR, corner_x_FL, corner_x_RL]).T
        corners_y = np.vstack([corner_y_RL, corner_y_RR, corner_y_FR, corner_y_FL, corner_y_RL]).T
        scan_indexes = data_df.loc[:, "scan_index"].to_numpy()
        for row, scan_index in enumerate(scan_indexes):
            self.x_data[scan_index] = corners_x[row, :]
            self.y_data[scan_index] = corners_y[row, :]
            self.is_visible[scan_index] = np.array([True]*5)
            self.df_indexes[scan_index] = np.array([scan_index]*5)