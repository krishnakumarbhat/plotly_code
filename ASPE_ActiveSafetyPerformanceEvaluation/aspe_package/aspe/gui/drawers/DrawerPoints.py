import numpy as np
import pyqtgraph as pg

from aspe.gui.drawers.abstract_drawers.DrawerLeaf import DrawerLeaf
from aspe.gui.views.settings.PointsDrawerSettingsWidget import PointsDrawerSettingsWidget


class DrawerPoints(DrawerLeaf):
    def __init__(self, parent, name, plot_item: pg.PlotItem, color: str, symbol: str, symbol_size: float):
        super().__init__(parent, name, plot_item, color)
        self.symbol = symbol
        self.symbol_size = symbol_size
        self.graphic_object = self.plot_item.plot([], [], pen=None, symbolBrush=self.color,
                                                  symbol=self.symbol, symbolSize=self.symbol_size)
        self.settings_widget = None
        self.graphic_object.scatter.drawer = self  # trick for handling clicking events, check BirdsEyeView class

    def set_data(self, data_df, x_signal_signature, y_signal_signature):
        if "df_indexes" not in data_df.columns:
            data_df["df_indexes"] = data_df.index
        data_df_grouped = data_df.groupby(by="scan_index")
        self.x_data = data_df_grouped[x_signal_signature].apply(np.array).to_dict()
        self.y_data = data_df_grouped[y_signal_signature].apply(np.array).to_dict()
        self.df_indexes = data_df_grouped["df_indexes"].apply(np.array).to_dict()

    def plot_scan_index(self, scan_index):
        if self.is_enabled:
            try:
                x = self.x_data[scan_index]
                y = self.y_data[scan_index]
                self.graphic_object.setData(y, x)  # VCS
            except KeyError:
                self.plot_empty_data()

    def update_symbol(self, symbol):
        self.symbol = symbol
        self.update_graphic_object_style()

    def update_symbol_size(self, symbol_size):
        self.symbol_size = symbol_size
        self.update_graphic_object_style()

    def update_graphic_object_style(self):
        self.graphic_object.setSymbolBrush(self.color)
        self.graphic_object.setSymbol(self.symbol)
        self.graphic_object.setSymbolSize(self.symbol_size)

    def create_settings_widget(self):
        self.settings_widget = PointsDrawerSettingsWidget(self.name, self)
        self.update_setting_widget()
        return self.settings_widget

    def get_df_index_by_position(self, position_x, position_y):
        current_x_points = self.x_data[self.parent.current_scan_index]
        current_y_points = self.y_data[self.parent.current_scan_index]
        current_df_idxs = self.df_indexes[self.parent.current_scan_index]
        target_idx = \
            np.sqrt(np.square(position_x - current_x_points) + np.square(position_y - current_y_points)).argmin()
        return current_df_idxs[target_idx]

    def get_state(self):
        return {
            "is_enabled": self.is_enabled,
            "color": self.color,
            "symbol": self.symbol,
            "symbol_size": self.symbol_size,
        }

    def load_state(self, state):
        self.is_enabled = state["is_enabled"]
        self.color = state["color"]
        self.symbol = state["symbol"]
        self.symbol_size = state["symbol_size"]
        self.settings_widget.load_state(state)
        self.update_graphic_object_style()
        if not self.is_enabled:
            self.disable()

    def update_setting_widget(self):
        state = self.get_state()
        self.settings_widget.load_state(state)

    def clear(self):
        self.graphic_object.scatter.drawer = None
        super().clear()
