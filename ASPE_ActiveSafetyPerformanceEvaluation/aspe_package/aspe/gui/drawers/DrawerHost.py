
import math

import numpy as np
import pandas as pd

from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.DrawerBBoxes import DrawerBBoxes
from aspe.gui.drawers.DrawerCircle import DrawerCircle
from aspe.gui.drawers.DrawerHostBBox import DrawerHostBBox
from aspe.gui.drawers.DrawerVector import DrawerVector


class DrawerHost(DrawerComposite):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)
        self.data_set.signals["center_x"] = -2.5
        self.data_set.signals["center_y"] = 0.0
        self.data_set.signals["bounding_box_orientation"] = 0.0
        self.data_set.signals["bounding_box_dimensions_x"] = 5.0
        self.data_set.signals["bounding_box_dimensions_y"] = 2.5

    def create_selection_drawer(self, plot_item):
        return DrawerBBoxes(self, "HostSelection", plot_item, color="#ff0000", line_style="--", line_width=2)

    def create_drawers(self, plot_item):
        host_bbox = DrawerHostBBox(self, "BBox", plot_item, color="#008080")
        velocities_drawer = DrawerVector(self,
                                         "Velocity",
                                         plot_item,
                                         color="#3264C8",
                                         line_style="-",
                                         line_width=1)
        predicted_path_drawer = DrawerCircle(self,
                                             "PredictedPath"
                                             , plot_item,
                                             color="#cccc00c8",
                                             line_style="-",
                                             line_width=1)
        width = self.data_set.bounding_box_dimensions_x
        length = self.data_set.bounding_box_dimensions_y
        if width is None or math.isnan(width):
            width = 2.0  # TODO FZD-376: this should be handled by extractor
        if length is None or math.isnan(length):
            length = 5.0

        host_bbox.set_data(self.data_set.signals, "position_x", "position_y", length, width)
        velocities_drawer.set_data(self.data_set.signals,
                                   "position_x",
                                   "position_y",
                                   "velocity_otg_x",
                                   "velocity_otg_y")
        predicted_path_drawer.set_data(calc_data_set_for_predicted_path(self.data_set.signals),
                                       "center_x",
                                       "center_y",
                                       "curvature",
                                       max_x_value= 130, min_x_value=-50, max_theta=np.pi, line_points=300)
        return [velocities_drawer, predicted_path_drawer, host_bbox]

    def on_click(self, clicked_drawer, click_event):
        if clicked_drawer is self.selection_drawer:
            self.deselect()
        else:
            selected_df_index = \
                self.data_set.signals.loc[
                                self.data_set.signals.loc[:, "scan_index"] == self.current_scan_index, :].index[0]
            if selected_df_index is not None:
                self.select(selected_df_index)

    def select(self, df_index):
        selected_row = self.data_set.signals.loc[df_index]
        if self.selected_unique_id != selected_row.unique_id:
            self.selected_unique_id = selected_row.unique_id
            selected_df = self.data_set.signals.loc[
                          self.data_set.signals.loc[:, "unique_id"] == self.selected_unique_id, :]
            if self.data_set.raw_signals is not None:
                raw_df = self.data_set.raw_signals.loc[selected_df.index, :]
                selected_df = selected_df.join(raw_df, rsuffix="_raw")
            self.selected_data = selected_df
            self.selection_drawer.set_data(selected_df, "center_x", "center_y", "bounding_box_orientation",
                                           "bounding_box_dimensions_x", "bounding_box_dimensions_y")
            self.selection_drawer.plot_scan_index(self.current_scan_index)
            self.parent.on_select(self)
        else:
            self.deselect()


def calc_data_set_for_predicted_path(data_model):
    # assigning the necessary variables from extracted data
    w_yaw_rate = data_model.yaw_rate.to_numpy()  # angular velocity
    v_raw_speed = data_model.raw_speed.to_numpy()  # linear velocity

    # calculate radius of host predicted path
    epsilon = 1e-5
    # if host is not moving, radius=100000 which means drawing straight predicted path
    radius = np.array([100000] * w_yaw_rate.size)
    valid_samples = abs(w_yaw_rate) > epsilon
    radius[valid_samples] = -v_raw_speed[valid_samples] / w_yaw_rate[abs(w_yaw_rate) > epsilon]

    # calculations of the circles centre
    center_x = [0] * radius.size
    center_y = -radius

    return pd.DataFrame({"scan_index": data_model.scan_index,
                         "center_x": center_x,
                         "center_y": center_y,
                         "curvature": radius})
