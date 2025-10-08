import numpy as np
import pyqtgraph as pg

from aspe.gui.drawers.abstract_drawers.DrawerLeaf import DrawerLeaf
from aspe.gui.views.settings.LinesDrawerSettingsWidget import LinesDrawerSettingsWidget


class DrawerLines(DrawerLeaf):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str, line_style: str, line_width: float):
        super().__init__(parent, name, plot_item, color)
        self.line_style = line_style
        self.line_width = line_width
        self.graphic_object = self.plot_item.plot([], [], pen=pg.mkPen(color=self.color, width=line_width,
                                                                       style=get_qt_line_style(self.line_style)))
        self.graphic_object.curve.drawer = self  # trick for handling clicking events, check BirdsEyeView class
        self.graphic_object.curve.setClickable(True)

    def plot_scan_index(self, scan_index):
        self.current_scan_index = scan_index
        if self.is_enabled:
            try:
                x = self.x_data[scan_index]
                y = self.y_data[scan_index]
                visible_mask = self.is_visible[scan_index]
                self.graphic_object.setData(y, x, connect=visible_mask)
            except KeyError:
                self.plot_empty_data()

    def change_line_style(self, line_style_str):
        self.line_style = line_style_str
        self.update_graphic_object_style()

    def change_line_width(self, line_width):
        self.line_width = line_width
        self.update_graphic_object_style()

    def update_graphic_object_style(self):
        pen = pg.mkPen(color=self.color, width=self.line_width,
                       style=get_qt_line_style(self.line_style))
        self.graphic_object.setPen(pen)

    def create_settings_widget(self):
        self.settings_widget = LinesDrawerSettingsWidget(self.name, self)
        self.update_setting_widget()
        return self.settings_widget

    def get_df_index_by_position(self, x_0, y_0):
        """
        https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
        :param mouse_click_event:
        :return:
        """
        current_df_idxs = self.df_indexes[self.parent.current_scan_index]
        x_1 = self.x_data[self.parent.current_scan_index]
        y_1 = self.y_data[self.parent.current_scan_index]

        x_2 = np.roll(x_1, -1)
        y_2 = np.roll(y_1, -1)

        # Velocity example (2 point lines)
        # x = [x1, x2, nan, x3, x4, nan ...]
        # x_1 = x1 x2 nan x3 x4 nan ...
        # x_2 = x2 nan x3 x4 nan x5 ...
        # remove nans to avoid warnings

        is_not_nan_mask = ~np.isnan(x_1) & ~np.isnan(x_2)
        x_1 = x_1[is_not_nan_mask]
        y_1 = y_1[is_not_nan_mask]
        x_2 = x_2[is_not_nan_mask]
        y_2 = y_2[is_not_nan_mask]
        current_df_idxs = current_df_idxs[is_not_nan_mask]

        # x_1 = x1 x3 ...
        # x_2 = x2 x4 ...
        # filter only lines for which clicked point is within it range, epsilon is included to handle 0 or 90 deg
        # oriented lines

        epsilon = 0.1
        mask_x = ((x_1 - epsilon < x_0) & (x_0 < x_2 + epsilon)) | ((x_2 - epsilon < x_0) & (x_0 < x_1 + epsilon))
        mask_y = ((y_1 - epsilon < y_0) & (y_0 < y_2 + epsilon)) | ((y_2 - epsilon < y_0) & (y_0 < y_1 + epsilon))
        in_boundary_mask = mask_x & mask_y
        x_1 = x_1[in_boundary_mask]
        y_1 = y_1[in_boundary_mask]
        x_2 = x_2[in_boundary_mask]
        y_2 = y_2[in_boundary_mask]
        current_df_idxs = current_df_idxs[in_boundary_mask]
        distances = np.abs((y_2 - y_1)*x_0 - (x_2 - x_1)*y_0 + x_2*y_1 - y_2*x_1) / np.sqrt((np.square(y_2 - y_1) +
                                                                                             np.square(x_2 - x_1)))
        target_df_idx = None
        if distances.shape[0] > 0:
            min_dist_idx = distances.argmin()
            target_df_idx = current_df_idxs[min_dist_idx]
        return target_df_idx

    def get_state(self):
        return {
            "is_enabled": self.is_enabled,
            "color": self.color,
            "line_style": self.line_style,
            "line_width": self.line_width,
        }

    def load_state(self, state):
        self.is_enabled = state["is_enabled"]
        self.color = state["color"]
        self.line_style = state["line_style"]
        self.line_width = state["line_width"]
        self.settings_widget.load_state(state)
        self.update_graphic_object_style()
        if not self.is_enabled:
            self.disable()

    def update_setting_widget(self):
        state = self.get_state()
        self.settings_widget.load_state(state)

    def clear(self):
        self.graphic_object.curve.drawer = None
        super().clear()


def get_qt_line_style(style_str: str):
    if style_str == "--":
        style = pg.QtCore.Qt.DashLine
    elif style_str == "-.-":
        style = pg.QtCore.Qt.DashDotLine
    elif style_str == "...":
        style = pg.QtCore.Qt.DotLine
    elif style_str == "-..-":
        style = pg.QtCore.Qt.DashDotDotLine
    else:
        style = pg.QtCore.Qt.SolidLine
    return style