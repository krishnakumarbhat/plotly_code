import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.gui.drawers.DrawerLines import DrawerLines
from aspe.gui.utilities.calc_circle_points import calc_circle_points


class DrawerCircle(DrawerLines):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str, line_style: str, line_width: float):
        super().__init__(parent, name, plot_item, color, line_style, line_width)

    def set_data(self, data_df, center_x_signature, center_y_signature, radius_signature, max_x_value, min_x_value,
                 line_points = 50, circle_radius_for_straight_line = 8000,  max_theta=2*np.pi):
        circuit_x, circuit_y = calc_circle_points(data_df.loc[:, center_x_signature],
                                                  data_df.loc[:, center_y_signature],
                                                  data_df.loc[:, radius_signature],
                                                  max_x_value,
                                                  min_x_value,
                                                  line_points,
                                                  circle_radius_for_straight_line,
                                                  max_theta)
        scan_index_raw = data_df.loc[:, "scan_index"].to_numpy()
        df_indexes_raw = data_df.index.to_numpy()
        scan_index_repeated = np.tile(scan_index_raw, (line_points+2, 1)).T.reshape(-1)
        df_index_repeated = np.tile(df_indexes_raw, (line_points+2, 1)).T.reshape(-1)

        circle_points = pd.DataFrame({"circuit_x": circuit_x.astype(np.float32),
                                "circuit_y": circuit_y.astype(np.float32),
                                "scan_index": scan_index_repeated,
                                "df_indexes": df_index_repeated})
        #filtering Nan values
        circle_points["is_nan"] = ~( (np.isnan(circle_points["circuit_x"])) & ~np.isnan(circle_points["circuit_y"]) )
        circle_points = circle_points.loc[circle_points["is_nan"] > 0]
        circle_points = circle_points.drop("is_nan", axis=1)
        nan_mask = ~(circle_points.duplicated() & pd.isnull(circle_points.loc[:, "circuit_x"]))
        circle_points = circle_points.loc[nan_mask, :]
        circle_points["connect_mask"] = ~(np.isnan(circle_points["circuit_x"]) & np.isnan(circle_points["circuit_y"]))
        circle_points_grouped = circle_points.groupby(by="scan_index")
        self.x_data = circle_points_grouped["circuit_x"].apply(np.array).to_dict()
        self.y_data = circle_points_grouped["circuit_y"].apply(np.array).to_dict()
        self.is_visible = circle_points_grouped["connect_mask"].apply(np.array).to_dict()
        self.df_indexes = circle_points_grouped["df_indexes"].apply(np.array).to_dict()
