import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.gui.drawers.DrawerLines import DrawerLines
from aspe.gui.utilities.calc_bbox_corners import calc_bbox_corners


class DrawerRelevancy(DrawerLines):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, associated_data):
        super().__init__(parent, name, plot_item, "#ff0000", "--", 1)
        self.line_points = 2
        self.set_data(associated_data)
        self.settings_widget = self.create_settings_widget()

    def set_data(self, data_df):
        corners_x, corners_y = calc_bbox_corners(data_df.loc[:, "center_x"], data_df.loc[:, "center_y"],
                                                 data_df.loc[:, "bounding_box_orientation"],
                                                 data_df.loc[:, "bounding_box_dimensions_x"],
                                                 data_df.loc[:, "bounding_box_dimensions_y"])
        only_diagonal_indexes = np.full(corners_x.shape, False)
        only_diagonal_indexes[0::6] = True
        only_diagonal_indexes[2::6] = True
        only_diagonal_indexes[5::6] = True
        corners_x = corners_x[only_diagonal_indexes]
        corners_y = corners_y[only_diagonal_indexes]

        scan_index_raw = data_df.loc[:, "scan_index"].to_numpy()
        df_indexes_raw = data_df.index.to_numpy()
        scan_index_repeated = np.tile(scan_index_raw, (3, 1)).T.reshape(-1)
        df_index_repeated = np.tile(df_indexes_raw, (3, 1)).T.reshape(-1)

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

    def on_click(self, mouse_click_event):
        pass  # do nothing