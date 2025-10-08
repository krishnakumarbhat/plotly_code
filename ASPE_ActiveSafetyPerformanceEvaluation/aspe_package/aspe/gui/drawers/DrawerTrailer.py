from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.DrawerBBoxes import DrawerBBoxes
from aspe.gui.drawers.DrawerTrailerBBox import DrawerTrailerBBox


class DrawerTrailer(DrawerComposite):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)

    def create_selection_drawer(self, plot_item):
        return DrawerBBoxes(self, "TrailerSelection", plot_item, color="#ff0000", line_style="--", line_width=2)

    def create_drawers(self, plot_item):
        trailer_bounding_boxes_drawer = DrawerTrailerBBox(self, "BBox", plot_item, color="#00FFFF", line_style="-",
                                                          line_width=3)

        trailer_bounding_boxes_drawer.set_data(self.data_set.signals,
                                               "position_x",
                                               "position_y",
                                               "bounding_box_orientation",
                                               "bounding_box_dimensions_x",
                                               "bounding_box_dimensions_y")

        return [trailer_bounding_boxes_drawer]

    def on_click(self, clicked_drawer, click_event):
        if clicked_drawer is self.selection_drawer:
            self.deselect()
        else:
            selected_df_index = \
                self.data_set.signals.loc[
                self.data_set.signals.loc[:, "scan_index"] == self.current_scan_index, :].index[0]
            if selected_df_index is not None:
                self.select(selected_df_index)

    def select(self, df_index):
        selected_row = self.data_set.signals.loc[df_index]
        if self.selected_unique_id != selected_row.unique_id:
            self.selected_unique_id = selected_row.unique_id
            selected_df = self.data_set.signals.loc[
                          self.data_set.signals.loc[:, "unique_id"] == self.selected_unique_id, :]
            if self.data_set.raw_signals is not None:
                raw_df = self.data_set.raw_signals.loc[selected_df.index, :]
                selected_df = selected_df.join(raw_df, rsuffix="_raw")
            self.selected_data = selected_df
            self.selection_drawer.set_data(selected_df, "center_x", "center_y", "bounding_box_orientation",
                                           "bounding_box_dimensions_x", "bounding_box_dimensions_y")
            self.selection_drawer.plot_scan_index(self.current_scan_index)
            self.parent.on_select(self)
        else:
            self.deselect()
