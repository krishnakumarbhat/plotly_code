import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.gui.drawers.DrawerFilledPolygon import DrawerFilledPolygon
from aspe.gui.utilities.calc_fov_points import calc_fov_points
from aspe.utilities.SupportingFunctions import add_alpha_to_rgb


class DrawerSensorFoV(DrawerFilledPolygon):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str):
        super().__init__(parent, name, plot_item, color)
        self.line_points = 25
        self.alpha = "14"  # alpha channel value

    def update_graphic_object_style(self):
        color = add_alpha_to_rgb(color=self.color, alpha=self.alpha)
        brush = pg.mkBrush(color=color)
        self.graphic_object.setBrush(brush)

    def set_data(self, data_df):
        fov_x, fov_y = calc_fov_points(data_df.loc[:, "position_x"],
                                       data_df.loc[:, "position_y"],
                                       data_df.loc[:, "boresight_az_angle"],
                                       data_df.loc[:, "max_range"],
                                       data_df.loc[:, "min_azimuth"],
                                       data_df.loc[:, "max_azimuth"],
                                       line_points=self.line_points)
        scan_index_raw = data_df.loc[:, "scan_index"].to_numpy()
        df_indexes_raw = data_df.index.to_numpy()
        scan_index_repeated = np.tile(scan_index_raw, (self.line_points + 3, 1)).T.reshape(-1)
        df_index_repeated = np.tile(df_indexes_raw, (self.line_points + 3, 1)).T.reshape(-1)

        fov_points = pd.DataFrame({"fov_x": fov_x.astype(np.float32),
                                   "fov_y": fov_y.astype(np.float32),
                                   "scan_index": scan_index_repeated,
                                   "df_indexes": df_index_repeated})

        nan_mask = np.isnan(fov_points["fov_x"])
        fov_points["connect_mask"] = ~(nan_mask & np.roll(nan_mask, -1))
        fov_points_grouped = fov_points.groupby(by="scan_index")
        self.x_data = fov_points_grouped["fov_x"].apply(np.array).to_dict()
        self.y_data = fov_points_grouped["fov_y"].apply(np.array).to_dict()
        self.is_visible = fov_points_grouped["connect_mask"].apply(np.array).to_dict()
        self.df_indexes = fov_points_grouped["df_indexes"].apply(np.array).to_dict()

