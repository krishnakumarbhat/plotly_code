import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.gui.drawers.DrawerLines import DrawerLines
from aspe.gui.utilities.calc_bbox_corners import calc_bbox_corners


class DrawerBBoxes(DrawerLines):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str, line_style: str, line_width: float):
        super().__init__(parent, name, plot_item, color, line_style, line_width)
        self.line_points = 5

    def set_data(self,
                 data_df,
                 center_x_signature,
                 center_y_signature,
                 orientation_signature,
                 length_signature,
                 width_signature):
        corners_x, corners_y = calc_bbox_corners(
            data_df.loc[:, center_x_signature], data_df.loc[:, center_y_signature],
            data_df.loc[:, orientation_signature], data_df.loc[:, length_signature],
            data_df.loc[:, width_signature])
        scan_index_raw = data_df.loc[:, "scan_index"].to_numpy()
        df_indexes_raw = data_df.index.to_numpy()
        scan_index_repeated = np.tile(scan_index_raw, (6, 1)).T.reshape(-1)
        df_index_repeated = np.tile(df_indexes_raw, (6, 1)).T.reshape(-1)

        corners = pd.DataFrame({"corners_x": corners_x.astype(np.float32),
                                "corners_y": corners_y.astype(np.float32),
                                "scan_index": scan_index_repeated,
                                "df_indexes": df_index_repeated})
        nan_mask = np.isnan(corners.loc[:, "corners_x"].to_numpy())
        corners["connect_mask"] = ~(nan_mask | np.roll(nan_mask, -1))
        corners_grouped = corners.groupby(by="scan_index")
        self.x_data = corners_grouped["corners_x"].apply(np.array).to_dict()
        self.y_data = corners_grouped["corners_y"].apply(np.array).to_dict()
        self.is_visible = corners_grouped["connect_mask"].apply(np.array).to_dict()
        self.df_indexes = corners_grouped["df_indexes"].apply(np.array).to_dict()

    def append_legend_part(self, legend_df, legend_dict):
        if self.is_enabled:
            legend_dict["subdrawer_name"] = [self.name]
            legend_dict["color"] = [self.color]
            legend_dict["style"] = [self.line_style]
            legend_df = legend_df.append(pd.DataFrame(legend_dict))
        return legend_df

