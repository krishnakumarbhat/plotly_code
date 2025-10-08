import pandas as pd

from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.DrawerContour import DrawerContour
from aspe.gui.drawers.DrawerErrCovPosition import DrawerErrCovPosition
from aspe.gui.drawers.DrawerLabels import DrawerLabels
from aspe.gui.drawers.DrawerPoints import DrawerPoints


def ellipse_data_prepare(_, data_df, *args, **kwargs):
    """Monkey patching of DrawerErrCovPosition - used to prepare strictly SG data for the drawer"""
    return data_df.rename(
        columns={"position_variance_x": "var_x", "position_variance_y": "var_y", "position_covariance": "cov_xy"})


class DrawerStationaryGeometries(DrawerComposite):

    def create_selection_drawer(self, plot_item) -> DrawerPoints:
        return DrawerPoints(self, "SGSelection", plot_item, symbol="o", color="#ff0000", symbol_size=10)

    def create_drawers(self, plot_item) -> list:
        contour_line_drawer = DrawerContour(self, "Contour", plot_item, color="#AA0000", line_style="-", line_width=3)
        vertices_points_drawer = \
            DrawerPoints(self, "VertexPoint", plot_item, symbol="h", color="#800080", symbol_size=8)
        label_drawer = DrawerLabels(self, "SGLabels", plot_item, color="#6464C8")
        DrawerErrCovPosition.ellipse_data_prepare = ellipse_data_prepare
        ellipse_covariance_drawer = DrawerErrCovPosition(self, "ErrCov", plot_item, color="#ffff91", line_style="--",
                                                         line_width=1, extracted_data_set=self.data_set.signals)

        contour_line_drawer.set_data(self.data_set.signals, "position_x", "position_y")
        vertices_points_drawer.set_data(self.data_set.signals, "position_x", "position_y")
        label_drawer.set_data(self.data_set.signals, self.data_set.raw_signals, "position_x", "position_y")
        return [contour_line_drawer, vertices_points_drawer, label_drawer, ellipse_covariance_drawer]

    def select(self, df_index: int, clicked_drawer) -> None:
        selected_row = self.data_set.signals.loc[df_index, :]
        if self.selected_unique_id != selected_row.unique_id:
            self.selected_unique_id = selected_row.unique_id
            if type(clicked_drawer) == DrawerPoints:
                selected_df = self.data_set.signals.loc[[selected_row.df_indexes], :].copy()
            else:
                mask = ["position_x_agg", "position_y_agg", "position_variance_x", "position_variance_y",
                        "position_covariance", "df_indexes"]
                selected_df = \
                    self.data_set.signals.loc[self.data_set.signals["unique_id"] == self.selected_unique_id, :].copy()
                selected_df["position_x_agg"] = selected_df["position_x"]
                selected_df["position_y_agg"] = selected_df["position_y"]
                buff = selected_df.groupby("scan_index").agg(list).explode("df_indexes")[mask].set_index("df_indexes")
                selected_df.update(buff)
            if self.data_set.raw_signals is not None:
                raw_df = self.data_set.raw_signals.loc[selected_df.index, :]
                selected_df = selected_df.join(raw_df, rsuffix="_raw")
            self.selected_data = selected_df
            self.selection_drawer.set_data(self.selected_data, "position_x", "position_y")
            self.selection_drawer.plot_scan_index(self.current_scan_index)
            self.parent.on_select(self)
        else:
            self.deselect()

    def on_click(self, clicked_drawer, click_event):
        if clicked_drawer is self.selection_drawer:
            self.deselect()
        else:
            clicked_y, clicked_x = click_event.pos()
            selected_df_index = clicked_drawer.get_df_index_by_position(clicked_x, clicked_y)
            if selected_df_index is not None:
                self.select(selected_df_index, clicked_drawer)

    def append_legend_part(self, legend_df: pd.DataFrame, legend_dict: dict) -> pd.DataFrame:
        if self.is_enabled:
            legend_dict["drawer_name"] = self.name
            for drawer in self.drawers:
                legend_df = drawer.append_legend_part(legend_df, legend_dict)
        return legend_df
