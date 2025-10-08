import numpy as np

from aspe.gui.drawers.abstract_drawers.DrawerComponent import DrawerComponent
from aspe.gui.views.settings.DrawingSetSettingsWidget import DrawingSetSettingsWidget


class DrawerComposite(DrawerComponent):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, name)
        self.data_set = extracted_data_set
        self.is_enabled = True
        self.current_scan_index = None
        self.selected_unique_id = None
        self.selected_data = None
        self.settings_widget = None
        self.drawers = self.create_drawers(plot_item)
        # nasty solution, TODO FZD-377: use everywhere dataframe instead of DataSet
        if hasattr(self.data_set, "signals"):
            self.scan_indexes = np.unique(self.data_set.signals.scan_index.to_numpy())
        else:
            self.scan_indexes = np.unique(self.data_set.scan_index.to_numpy())
        self.selection_drawer = self.create_selection_drawer(plot_item)
        self.settings_widget = self.create_settings_widget()

    def create_selection_drawer(self, plot_item):
        raise NotImplementedError("Virtual method - implement it in child class")

    def create_drawers(self, plot_item):
        raise NotImplementedError("Virtual method - implement it in child class")

    def plot_scan_index(self, scan_index):
        self.current_scan_index = scan_index
        if self.is_enabled:
            for drawer in self.drawers:
                drawer.plot_scan_index(scan_index)
            if self.selected_unique_id is not None:
                self.update_selection()

    def plot_empty_data(self):
        for drawer in self.drawers:
            drawer.plot_empty_data()
        if self.selected_unique_id is not None:
            self.selection_drawer.plot_empty_data()

    def enable(self):
        self.is_enabled = True
        self.plot_scan_index(self.current_scan_index)

    def disable(self):
        self.is_enabled = False
        for drawer in self.drawers:
            drawer.plot_empty_data()
        if self.selected_unique_id is not None:
            self.deselect()

    def update_selection(self):
        if np.isin(self.current_scan_index, self.selected_data.loc[:, "scan_index"].to_numpy()):
            self.selection_drawer.plot_scan_index(self.current_scan_index)
        else:
            self.deselect()

    def deselect(self):
        self.selected_unique_id = None
        self.selected_data = None
        self.selection_drawer.x_data = {}
        self.selection_drawer.y_data = {}
        self.selection_drawer.plot_empty_data()
        self.parent.on_deselect()

    def on_click(self, clicked_drawer, click_event):
        if clicked_drawer is self.selection_drawer:
            self.deselect()
        else:
            clicked_y, clicked_x = click_event.pos()
            selected_df_index = clicked_drawer.get_df_index_by_position(clicked_x, clicked_y)
            if selected_df_index is not None:
                self.select(selected_df_index)

    def create_settings_widget(self):
        self.settings_widget = DrawingSetSettingsWidget(self)
        for drawer in self.drawers:
            self.settings_widget.add_drawer_setting(drawer.create_settings_widget())
        return self.settings_widget

    def clear(self):
        if self.selected_unique_id is not None:
            self.deselect()
        self.parent = None
        self.data_set = None
        for drawer in self.drawers:
            drawer.clear()
        self.settings_widget.remove()
        self.settings_widget = None
        self.selected_data = None
        if self.selection_drawer is not None:
            self.selection_drawer.clear()
            self.selection_drawer = None
        self.drawers = []

    def get_state(self):
        return {
            "name": self.name,
            "is_enabled": self.is_enabled,
            "children_states": {drawer.name: drawer.get_state() for drawer in self.drawers},
        }

    def load_state(self, state):
        self.is_enabled = state.get("is_enabled", True)
        if "children_states" in state:
            for drawer_name, drawer_state in state["children_states"].items():
                drawer = self._get_drawer_by_name(drawer_name)
                if drawer is not None:
                    drawer.load_state(drawer_state)
        if not self.is_enabled:
            self.disable()
        else:
            self.enable()

        self.settings_widget.load_state(state)

    def _get_drawer_by_name(self, name):
        searched_drawer = None
        for drawer in self.drawers:
            if drawer.name == name:
                searched_drawer = drawer
                break
        return searched_drawer

