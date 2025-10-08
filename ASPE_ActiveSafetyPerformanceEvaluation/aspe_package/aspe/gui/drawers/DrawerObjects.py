
from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.DrawerBBoxes import DrawerBBoxes
from aspe.gui.drawers.DrawerErrCovPosition import DrawerErrCovPosition
from aspe.gui.drawers.DrawerErrCovVelocity import DrawerErrCovVelocity
from aspe.gui.drawers.DrawerLabels import DrawerLabels
from aspe.gui.drawers.DrawerPoints import DrawerPoints
from aspe.gui.drawers.DrawerVector import DrawerVector


class DrawerObjects(DrawerComposite):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)

    def create_selection_drawer(self, plot_item):
        return DrawerBBoxes(self, "ObjectSelection", plot_item, color="#ff0000", line_style="--", line_width=2)

    def create_drawers(self, plot_item):
        reference_points_drawer = DrawerPoints(self, "Centroid", plot_item, symbol="d", color="#808080", symbol_size=8)
        bounding_boxes_drawer = DrawerBBoxes(self, "BBox", plot_item, color="#008080", line_style="-", line_width=3)
        velocities_drawer = DrawerVector(self, "Velocity", plot_item, color="#3264C8", line_style="-", line_width=1)
        label_drawer = DrawerLabels(self, "Labels", plot_item, color="#6464C8")

        bounding_boxes_drawer.set_data(self.data_set.signals,
                                       "center_x",
                                       "center_y",
                                       "bounding_box_orientation",
                                       "bounding_box_dimensions_x",
                                       "bounding_box_dimensions_y")
        reference_points_drawer.set_data(self.data_set.signals, "position_x", "position_y")
        velocities_drawer.set_data(self.data_set.signals,
                                   "position_x",
                                   "position_y",
                                   "velocity_otg_x",
                                   "velocity_otg_y")

        drawers = [bounding_boxes_drawer, reference_points_drawer, velocities_drawer]

        if ("acceleration_otg_x" in self.data_set.signals) and ("acceleration_otg_x" in self.data_set.signals):
            accelerations_drawer = DrawerVector(self,
                                                "Acceleration",
                                                plot_item,
                                                color="#AA0078",
                                                line_style="-",
                                                line_width=1)
            accelerations_drawer.set_data(self.data_set.signals,
                                          "position_x",
                                          "position_y",
                                          "acceleration_otg_x",
                                          "acceleration_otg_y")
            accelerations_drawer.disable()
            drawers.append(accelerations_drawer)

        if "motion_model_variances_suppl" in self.data_set.signals and \
                not (self.data_set.signals["motion_model_variances_suppl"].isnull().all() or
                     self.data_set.signals["motion_model_covariances_suppl"].isnull().all()):
            ellipse_covariance_position_drawer = DrawerErrCovPosition(self, "ErrCovPosition", plot_item,
                color="#ffff91", line_style="--", line_width=1, extracted_data_set=self.data_set.signals)
            ellipse_covariance_velocity_drawer = DrawerErrCovVelocity(self, "ErrCovVelocity", plot_item,
                color="#ff9191", line_style="--", line_width=1, extracted_data_set=self.data_set.signals)

            drawers.append(ellipse_covariance_position_drawer)
            drawers.append(ellipse_covariance_velocity_drawer)

        label_drawer.set_data(self.data_set.signals, self.data_set.raw_signals, "position_x", "position_y")
        drawers.append(label_drawer)  # label drawer should be visualized at bottom of DrawerSettings window
        return drawers

    def select(self, df_index):
        selected_row = self.data_set.signals.loc[df_index, :]
        if self.selected_unique_id != selected_row.unique_id:
            self.selected_unique_id = selected_row.unique_id
            selected_df = \
                self.data_set.signals.loc[self.data_set.signals.loc[:, "unique_id"] == self.selected_unique_id, :]
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

    def get_parent(self):
        return self.parent

    def append_legend_part(self, legend_df, legend_dict):
        if self.is_enabled:
            drawer_name = self.name
            legend_dict["drawer_name"] = drawer_name
            for drawer in self.drawers:
                legend_df = drawer.append_legend_part(legend_df, legend_dict)
        return legend_df

