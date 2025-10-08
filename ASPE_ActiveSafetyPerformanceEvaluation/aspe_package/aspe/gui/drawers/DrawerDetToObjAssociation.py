
from copy import deepcopy

import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.gui.drawers.DrawerLines import DrawerLines


def set_reduced_objects(extracted_data_set):
    try:
        reduced_ds = deepcopy(extracted_data_set)
        reduced_mask = reduced_ds.loc[:, "reduced_id"] != 0
        reduced_ds = extracted_data_set.loc[reduced_mask, :]
        return reduced_ds
    except KeyError:
        return extracted_data_set

class DrawerDetToObjAssociation(DrawerLines):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, associated_data):
        super().__init__(parent, name, plot_item, "#ff0000", "--", 1)
        self.line_points = 2
        self.set_data(associated_data)
        self.settings_widget = self.create_settings_widget()
        self.all_assoc = associated_data
        self.reduced_assoc = set_reduced_objects(associated_data)
        self.only_reduced_assoc = False

    def switch_to_reduced(self):
        self.only_reduced_assoc = True
        data_set = self.reduced_assoc
        self.set_data(data_set)
        self.plot_scan_index(self.current_scan_index)

    def switch_to_all_assoc(self):
        self.only_reduced_objects = False
        data_set = self.all_assoc
        self.set_data(data_set)
        self.plot_scan_index(self.current_scan_index)

    def set_data(self, data_df):
        if "scan_index" not in data_df.columns:
            data_df["scan_index"] = data_df.index.get_level_values(0)

        x_1 = data_df["position_x_obj"].apply(np.array)
        y_1 = data_df["position_y_obj"].apply(np.array)
        x_2 = data_df["position_x_det"].apply(np.array)
        y_2 = data_df["position_y_det"].apply(np.array)
        nan_vec = np.full(shape=len(x_1), fill_value=np.nan)

        x_data = np.vstack([x_1, x_2, nan_vec]).T.reshape(-1)
        y_data = np.vstack([y_1, y_2, nan_vec]).T.reshape(-1)
        df = pd.DataFrame({"scan_index": np.repeat(data_df["scan_index"], 3),
                           "x_data": x_data,
                           "y_data": y_data})
        nan_mask = np.isnan(df.loc[:, "x_data"].to_numpy())
        df["connect_mask"] = ~(nan_mask | np.roll(nan_mask, -1))

        df_grouped = df.groupby(by="scan_index")
        self.x_data = df_grouped["x_data"].apply(np.array).to_dict()
        self.y_data = df_grouped["y_data"].apply(np.array).to_dict()
        self.is_visible = df_grouped["connect_mask"].apply(np.array).to_dict()

def on_click(self, mouse_click_event):
        pass  # do nothing