import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.gui.drawers.DrawerLines import DrawerLines


class DrawerVector(DrawerLines):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str, line_style: str, line_width: float):
        super().__init__(parent, name, plot_item, color, line_style, line_width)
        self.line_points = 2

    def set_data(self,
                 data_df,
                 vector_origin_x_signature,
                 vector_origin_y_signature,
                 vector_x_len_signature,
                 vector_y_len_signature):
        x_1 = data_df.loc[:, vector_origin_x_signature].to_numpy().astype(np.float32)
        y_1 = data_df.loc[:, vector_origin_y_signature].to_numpy().astype(np.float32)

        vector_x_len = data_df.loc[:, vector_x_len_signature].to_numpy().astype(np.float32)
        vector_y_len = data_df.loc[:, vector_y_len_signature].to_numpy().astype(np.float32)

        x_2 = x_1 + vector_x_len
        y_2 = y_1 + vector_y_len

        nan_vec = np.full(shape=x_1.shape[0], fill_value=np.nan)

        x_data = np.vstack([x_1, x_2, nan_vec]).T.reshape(-1)
        y_data = np.vstack([y_1, y_2, nan_vec]).T.reshape(-1)

        scan_index_raw = data_df.loc[:, "scan_index"].to_numpy()
        df_indexes_raw = data_df.index.to_numpy()
        scan_index_repeated = np.tile(scan_index_raw, (3, 1)).T.reshape(-1)
        df_index_repeated = np.tile(df_indexes_raw, (3, 1)).T.reshape(-1)

        points = pd.DataFrame({"x_data": x_data.astype(np.float32),
                                "y_data": y_data.astype(np.float32),
                                "scan_index": scan_index_repeated,
                                "df_indexes": df_index_repeated})
        nan_mask = np.isnan(points.loc[:, "x_data"].to_numpy())
        points["connect_mask"] = ~(nan_mask | np.roll(nan_mask, -1))
        points_grouped = points.groupby(by="scan_index")
        self.x_data = points_grouped["x_data"].apply(np.array).to_dict()
        self.y_data = points_grouped["y_data"].apply(np.array).to_dict()
        self.is_visible = points_grouped["connect_mask"].apply(np.array).to_dict()
        self.df_indexes = points_grouped["df_indexes"].apply(np.array).to_dict()