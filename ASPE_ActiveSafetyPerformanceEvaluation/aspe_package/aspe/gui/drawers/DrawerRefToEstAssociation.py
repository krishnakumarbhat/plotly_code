
import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.gui.drawers.DrawerLines import DrawerLines


class DrawerRefToEstAssociation(DrawerLines):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, associated_data):
        super().__init__(parent, name, plot_item, "#ff0000", "--", 1)
        self.line_points = 2
        self.set_data(associated_data)
        self.settings_widget = self.create_settings_widget()
        self.selected_unique_id = None
        self.selection_drawer = DrawerSelection(self, "selection_drawer", self.plot_item)
        self.selected_data = None

    def set_data(self, data_df):
        data_df = data_df[data_df.is_associated == True]
        self.association_data = data_df
        data_df["df_indexes"] = data_df.index
        x_1 = data_df["center_x_est"].to_numpy().astype(np.float32)
        y_1 = data_df["center_y_est"].to_numpy().astype(np.float32)
        x_2 = data_df["center_x_ref"].to_numpy().astype(np.float32)
        y_2 = data_df["center_y_ref"].to_numpy().astype(np.float32)
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

    def plot_scan_index(self, scan_index):
        self.current_scan_index = scan_index
        if self.is_enabled:
            try:
                x = self.x_data[scan_index]
                y = self.y_data[scan_index]
                visible_mask = self.is_visible[scan_index]
                nan_mask = np.isnan(x)
                x[nan_mask] = 0.0
                y[nan_mask] = 0.0
                self.graphic_object.setData(y, x, connect=visible_mask)
            except KeyError:
                self.plot_empty_data()
            self.selection_drawer.plot_scan_index(scan_index)

    def on_click(self, mouse_click_event):
        clicked_y, clicked_x = mouse_click_event.pos()
        target_df_index = self.get_df_index_by_position(clicked_x, clicked_y)
        self.select(target_df_index)

    def select(self, target_df_index):
        selected_row = self.association_data.loc[target_df_index, :]
        if self.selected_unique_id == selected_row.unique_id:
            self.deselect()
        else:
            self.selected_unique_id = selected_row.unique_id
            selected_df = \
                self.association_data.loc[(self.association_data.loc[:, "unique_id"] == self.selected_unique_id), :]
            self.selected_data = selected_df
            self.selection_drawer.set_data(selected_df)
            self.selection_drawer.plot_scan_index(self.current_scan_index)
            self.parent.on_select(self)

    def deselect(self):
        if self.selection_drawer is not None:
            self.selection_drawer.plot_empty_data()
            self.selection_drawer.clear_data()
        self.selected_unique_id = None
        self.selected_data = None
        self.parent.on_deselect()

    def disable(self):
        self.is_enabled = False
        self.deselect()
        self.plot_empty_data()


class DrawerSelection(DrawerLines):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem):
        super().__init__(parent, name, plot_item, "#00ff00", "-", 1)
        self.line_points = 2

    def set_data(self, data_df):
        self.plot_empty_data()
        self.clear_data()
        for scan_index, scan_df in data_df.groupby(by="scan_index"):
            x_1 = scan_df.loc[:, "center_x_est"].to_numpy().astype(np.float32)
            y_1 = scan_df.loc[:, "center_y_est"].to_numpy().astype(np.float32)

            x_2 = scan_df.loc[:, "center_x_ref"].to_numpy().astype(np.float32)
            y_2 = scan_df.loc[:, "center_y_ref"].to_numpy().astype(np.float32)

            nan_vec = np.full(shape=x_1.shape[0], fill_value=np.nan)

            x_data = np.vstack([x_1, x_2, nan_vec]).T.reshape(-1)
            y_data = np.vstack([y_1, y_2, nan_vec]).T.reshape(-1)

            self.x_data[scan_index] = x_data
            self.y_data[scan_index] = y_data

            nan_mask = np.isnan(x_data)
            connect_mask = ~(nan_mask | np.roll(nan_mask, -1))
            self.is_visible[scan_index] = connect_mask

            df_indexes = scan_df.index.to_numpy()
            df_indexes_flat = np.tile(df_indexes, (self.line_points + 1, 1)).T.reshape(-1)
            self.df_indexes[scan_index] = df_indexes_flat

    def clear_data(self):
        self.x_data = {}
        self.y_data = {}
        self.df_indexes = {}
        self.is_visible = {}

    def on_click(self, mouse_click_event):
        self.parent.deselect()