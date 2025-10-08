import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.gui.drawers.DrawerLines import DrawerLines


class DrawerRangeRates(DrawerLines):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str, line_style: str, line_width: float):
        super().__init__(parent, name, plot_item, color, line_style, line_width)
        self.line_points = 2

    def set_data(self, data_df, vector_origin_x_signautre, vector_origin_y_signature, vector_magnitude_signature,
                 vector_orientation_signature):
        data_df["df_indexes"] = data_df.index
        x_1 = data_df[vector_origin_x_signautre].to_numpy().astype(np.float32)
        y_1 = data_df[vector_origin_y_signature].to_numpy().astype(np.float32)
        magnitudes = data_df[vector_magnitude_signature].to_numpy().astype(np.float32)
        orientations = data_df[vector_orientation_signature].to_numpy().astype(np.float32)
        magnitude_threshold = -50
        magnitudes[magnitudes < magnitude_threshold] = 0
        x_2 = magnitudes * np.cos(orientations) * 0.3 + x_1
        y_2 = magnitudes * np.sin(orientations) * 0.3 + y_1
        nan_vec = np.full(shape=len(x_1), fill_value=np.nan)
        x_data = np.vstack([x_1, x_2, nan_vec]).T.reshape(-1)
        y_data = np.vstack([y_1, y_2, nan_vec]).T.reshape(-1)
        df = pd.DataFrame({"scan_index": np.repeat(data_df["scan_index"], 3),
                           "x_data": x_data,
                           "y_data": y_data,
                           "df_indexes": np.repeat(data_df["df_indexes"], 3)})
        nan_mask = np.isnan(df.loc[:, "x_data"].to_numpy())
        df["connect_mask"] = ~(nan_mask | np.roll(nan_mask, -1))

        df_grouped = df.groupby(by="scan_index")
        self.x_data = df_grouped["x_data"].apply(np.array).to_dict()
        self.y_data = df_grouped["y_data"].apply(np.array).to_dict()
        self.is_visible = df_grouped["connect_mask"].apply(np.array).to_dict()
        self.df_indexes = df_grouped["df_indexes"].apply(np.array).to_dict()
