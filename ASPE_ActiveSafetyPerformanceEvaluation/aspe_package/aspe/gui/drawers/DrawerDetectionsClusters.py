
from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.DrawerLabels import DrawerLabels
from aspe.gui.drawers.DrawerPoints import DrawerPoints
from aspe.gui.drawers.DrawerRangeRates import DrawerRangeRates


class DrawerDetectionsClusters(DrawerComposite):
    def __init__(self, parent, extracted_data_set, plot_item, name, rrate_signal_name, azimuth_signal_name):
        self.rrate_signal_name = rrate_signal_name
        self.azimuth_signal_name = azimuth_signal_name
        super().__init__(parent, extracted_data_set, plot_item, name)

    def create_selection_drawer(self, plot_item):
        return DrawerPoints(self, "ClusterSelection", plot_item, symbol="s", color="#ff0000", symbol_size=10)

    def create_drawers(self, plot_item):
        cluster_points_drawer = DrawerPoints(self, "ClusterPoint", plot_item, symbol="s", color="#ffffff",
                                             symbol_size=8)
        range_rates_drawer = DrawerRangeRates(self, "RangeRate", plot_item, color="#fafafa", line_style="-",
                                              line_width=1)
        label_drawer = DrawerLabels(self, "Labels", plot_item, color="#fafafa")

        cluster_points_drawer.set_data(self.data_set.signals, "position_x", "position_y")
        range_rates_drawer.set_data(self.data_set.signals, "position_x", "position_y", self.rrate_signal_name,
                                    self.azimuth_signal_name)
        label_drawer.set_data(self.data_set.signals, self.data_set.raw_signals, "position_x", "position_y")

        return [cluster_points_drawer, range_rates_drawer, label_drawer]

    def select(self, df_index):
        selected_row = self.data_set.signals.loc[df_index, :]
        if selected_row.unique_id != self.selected_unique_id:
            self.selected_unique_id = selected_row.unique_id
            selected_df = \
                self.data_set.signals.loc[self.data_set.signals.loc[:, "unique_id"] == self.selected_unique_id, :]
            if self.data_set.raw_signals is not None:
                raw_df = self.data_set.raw_signals.loc[selected_df.index, :]
                selected_df = selected_df.join(raw_df, rsuffix="_raw")
            self.selected_data = selected_df
            self.selection_drawer.set_data(self.selected_data, "position_x", "position_y")
            self.selection_drawer.plot_scan_index(self.current_scan_index)
            self.parent.on_select(self)
        else:  #casewhenclickonsamedetection
            self.deselect()

