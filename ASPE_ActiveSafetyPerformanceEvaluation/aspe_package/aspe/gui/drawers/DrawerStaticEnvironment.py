
from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.Drawer2ndDegreePolynomial import Drawer2ndDegreePolynomial
from aspe.gui.drawers.DrawerLabels import DrawerLabels


class DrawerStaticEnvironment(DrawerComposite):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)

    def create_selection_drawer(self, plot_item):
        return Drawer2ndDegreePolynomial(self, "PolyLines", plot_item, color="#ff0000", line_width=2, line_style="-")

    def create_drawers(self, plot_item):
        polys_drawer = Drawer2ndDegreePolynomial(self,
                                                 "PolyLines",
                                                 plot_item,
                                                 color="#808080",
                                                 line_width=2,
                                                 line_style="-")
        label_drawer = DrawerLabels(self, "Labels", plot_item, color="#6464c8")

        polys_drawer.set_data(self.data_set.signals,
                              "polynomial_p2",
                              "polynomial_p1",
                              "polynomial_p0",
                              "x_min_limit",
                              "x_max_limit")

        self._add_labels_position_signals()
        label_drawer.set_data(self.data_set.signals, self.data_set.raw_signals, "x_label", "y_label")

        return [polys_drawer, label_drawer]

    def select(self, df_index):
        selected_row = self.data_set.signals.loc[df_index, :]
        if self.selected_unique_id != selected_row.unique_id:
            self.selected_unique_id = selected_row.unique_id
            selected_df = \
                self.data_set.signals.loc[self.data_set.signals.loc[:, "unique_id"] == self.selected_unique_id, :]
            if self.data_set.raw_signals is not None:
                raw_df = self.data_set.raw_signals.loc[selected_df.index, :]
                selected_df = selected_df.join(raw_df, rsuffix="_raw")
            self.selected_data = selected_df
            self.selection_drawer.set_data(self.selected_data,
                                           "polynomial_p2",
                                           "polynomial_p1",
                                           "polynomial_p0",
                                           "x_min_limit",
                                           "x_max_limit")
            self.selection_drawer.plot_scan_index(self.current_scan_index)
            self.parent.on_select(self)
        else:
            self.deselect()

    def get_parent(self):
        return self.parent

    def _add_labels_position_signals(self):
        """
        Add helper signals to define point where label should be placed.
        :return:
        """
        # min/max halfway
        x_label = (self.data_set.signals.loc[:, "x_min_limit"] + self.data_set.signals.loc[:, "x_max_limit"]) * 0.5
        p2 = self.data_set.signals.loc[:, "polynomial_p2"].to_numpy()
        p1 = self.data_set.signals.loc[:, "polynomial_p1"].to_numpy()
        p0 = self.data_set.signals.loc[:, "polynomial_p0"].to_numpy()

        y_label = (x_label * x_label) * p2 + x_label * p1 + p0
        self.data_set.signals["x_label"] = x_label
        self.data_set.signals["y_label"] = y_label
