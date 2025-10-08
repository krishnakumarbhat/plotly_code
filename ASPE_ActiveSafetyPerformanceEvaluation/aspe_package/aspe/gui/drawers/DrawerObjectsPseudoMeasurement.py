
from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.DrawerLabels import DrawerLabels
from aspe.gui.drawers.DrawerPoints import DrawerPoints


class DrawerObjectsPseudoMeasurement(DrawerComposite):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)

    def create_selection_drawer(self, plot_item):
        return DrawerPoints(self, "PseudoPositionSelection", plot_item, symbol="x", color="#ff0000", symbol_size=10)

    def create_drawers(self, plot_item):
        pseudo_position_drawer = DrawerPoints(
            self, "PseudoPosition", plot_item, symbol="x", color="#ff00ff", symbol_size=8)

        label_drawer = DrawerLabels(self, "Labels", plot_item, color="#6464C8")

        pseudo_position_drawer.set_data(self.data_set.signals, "pseudo_position_x", "pseudo_position_y")

        label_drawer.set_data(self.data_set.signals,
                              self.data_set.raw_signals,
                              "pseudo_position_x",
                              "pseudo_position_y")

        return [pseudo_position_drawer, label_drawer]

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
            self.selection_drawer.set_data(self.selected_data, "pseudo_position_x", "pseudo_position_y")
            self.selection_drawer.plot_scan_index(self.current_scan_index)
            self.parent.on_select(self)
        else:  #casewhenclickonsamedetection
            self.deselect()

