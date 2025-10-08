import numpy as np

from aspe.gui.views.settings.DataSourceSettingsWidget import DataSourceSettingsWidget


class DrawerExtractedData:
    def __init__(self, name, drawers_controller, data_model):
        self.name = name
        self.drawers_controller = drawers_controller
        self.data_model = data_model
        self.drawers = self.create_drawers()
        self.scan_indexes = self.get_scan_indexes()
        self.current_scan_index = 0
        self.settings_widget = self.create_settings_widget()
        self.is_enabled = True
        self.log_index = None

    def create_drawers(self):
        raise NotImplementedError("Virtual method - must be implemented")

    def get_scan_indexes(self):
        scan_indexes = []
        for drawer in self.drawers:
            if hasattr(drawer, "scan_indexes"):
                data_set_indexes = drawer.scan_indexes
                scan_indexes.append(data_set_indexes)
        return np.unique(np.hstack(scan_indexes))

    def plot_scan_index(self, scan_index):
        self.current_scan_index = scan_index
        if np.isin(scan_index, self.scan_indexes):
            for data_set in self.drawers:
                data_set.plot_scan_index(scan_index)
        else:
            for data_set in self.drawers:
                data_set.plot_empty_data()

    def create_settings_widget(self):
        settings_widget = DataSourceSettingsWidget(self)
        for drawer in self.drawers:
            settings_widget.add_data_set_settings(drawer.settings_widget, drawer.name)
        return settings_widget

    def enable(self):
        for drawer in self.drawers:
            drawer.enable()

    def disable(self):
        for drawer in self.drawers:
            drawer.disable()

    def on_select(self, selected_drawer):
        self.drawers_controller.on_select(self, selected_drawer)

    def on_deselect(self):
        self.drawers_controller.on_deselect()

    def clear(self):
        for drawer in self.drawers:
            drawer.clear()
        self.drawers = []
        self.drawers_controller = None
        self.data_model = None
        self.scan_indexes = None
        self.settings_widget.remove()
        self.settings_widget = None

    def get_state(self):
        return {
            "name": self.name,
            "is_enabled": self.is_enabled,
            "log_index": self.log_index,
            "children_states": {drawer.name: drawer.get_state() for drawer in self.drawers},
        }

    def load_state(self, state):
        self.is_enabled = state.get("is_enabled", True)
        self.log_index = state.get("log_index")
        if "children_states" in state:
            for drawer_name, drawer_state in state["children_states"].items():
                drawer = self._get_drawer_by_name(drawer_name)
                if drawer is not None:
                    drawer.load_state(drawer_state)
        return drawer

    def _get_drawer_by_name(self, name):
        searched_drawer = None
        for drawer in self.drawers:
            if drawer.name == name:
                searched_drawer = drawer
                break
        return searched_drawer
