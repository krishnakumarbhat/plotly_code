
from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.DrawerPoints import DrawerPoints
from aspe.gui.drawers.DrawerVector import DrawerVector


class DrawerObjectsPredictedState(DrawerComposite):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)

    def create_selection_drawer(self, plot_item):
        return DrawerPoints(
            self, "PredictedPositionSelection", plot_item, symbol="x", color="#ff0000", symbol_size=10)

    def create_drawers(self, plot_item):
        predicted_position_drawer = DrawerPoints(
            self, "PredictedPosition", plot_item, symbol="x", color="#ffff00", symbol_size=8)
        predicted_velocity_drawer = DrawerVector(
            self, "PredictedVelocity", plot_item, color="#ff8000", line_style="-", line_width=1)

        predicted_position_drawer.set_data(self.data_set.signals, "predicted_position_x", "predicted_position_y")
        predicted_velocity_drawer.set_data(self.data_set.signals,
                                           "predicted_position_x",
                                           "predicted_position_y",
                                           "predicted_velocity_x",
                                           "predicted_velocity_y")

        return [predicted_position_drawer, predicted_velocity_drawer]

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
            self.selection_drawer.set_data(self.selected_data, "predicted_position_x", "predicted_position_y")
            self.selection_drawer.plot_scan_index(self.current_scan_index)
            self.parent.on_select(self)
        else:  #casewhenclickonsamedetection
            self.deselect()

