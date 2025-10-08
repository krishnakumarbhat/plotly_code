from copy import deepcopy

from aspe.gui.drawers.DrawerObjects import DrawerObjects
from aspe.gui.views.settings.DrawerInternalObjectsSettings import DrawerInternalObjectsSettings


def set_reduced_objects(extracted_data_set):
    try:
        reduced_ds = deepcopy(extracted_data_set)
        reduced_mask = extracted_data_set.signals.loc[:, "reduced_id"] != 0
        reduced_ds.signals = extracted_data_set.signals.loc[reduced_mask, :]
        if reduced_ds.raw_signals is not None:
            reduced_ds.raw_signals = extracted_data_set.raw_signals.loc[reduced_mask, :]
        return reduced_ds

    except KeyError:
        return extracted_data_set


class DrawerInternalObjects(DrawerObjects):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)
        self.all_objects = extracted_data_set
        self.reduced_objects = set_reduced_objects(extracted_data_set)
        self.only_reduced_objects = False

    def switch_to_reduced(self):
        self.only_reduced_objects = True
        self.data_set = self.reduced_objects
        self.set_drawers_data()
        self.plot_scan_index(self.current_scan_index)

        self.parent.switch_to_reduced() # nasty solution

    def switch_to_all_objects(self):
        self.only_reduced_objects = False
        self.data_set = self.all_objects
        self.set_drawers_data()
        self.plot_scan_index(self.current_scan_index)

        self.parent.switch_to_all_data() # nasty solution

    def set_drawers_data(self):
        state = self.get_state()
        if "children_states" in state:
            for drawer_name, _ in state["children_states"].items():
                drawer = self._get_drawer_by_name(drawer_name)
                if drawer is not None:
                    if drawer_name == "BBox":
                        drawer.set_data(self.data_set.signals, "center_x", "center_y", "bounding_box_orientation",
                                             "bounding_box_dimensions_x", "bounding_box_dimensions_y")
                    elif drawer_name == "Centroid":
                        drawer.set_data(self.data_set.signals, "position_x", "position_y")
                    elif drawer_name == "Velocity":
                        drawer.set_data(
                            self.data_set.signals, "position_x", "position_y", "velocity_otg_x", "velocity_otg_y")
                    elif drawer_name == "Acceleration":
                        drawer.set_data(self.data_set.signals,
                                        "position_x",
                                        "position_y",
                                        "acceleration_otg_x",
                                        "acceleration_otg_y")
                    elif drawer_name == "Labels":
                        drawer.set_data(self.data_set.signals, self.data_set.raw_signals, "position_x", "position_y")
                    elif drawer_name == "ErrCovPosition" or drawer_name == "ErrCovVelocity":
                        drawer.set_data(self.data_set.signals)

    def create_settings_widget(self):
        self.settings_widget = DrawerInternalObjectsSettings(self)
        for drawer in self.drawers:
            self.settings_widget.add_drawer_setting(drawer.create_settings_widget())
        return self.settings_widget

    def get_state(self):
        return {
            "name": self.name,
            "is_enabled": self.is_enabled,
            "only_reduced_objects": self.only_reduced_objects,
            "children_states": {drawer.name: drawer.get_state() for drawer in self.drawers},
        }

    def load_state(self, state):
        self.is_enabled = state.get("is_enabled", True)
        self.only_reduced_objects = state.get("only_reduced_objects", False)
        if "children_states" in state:
            for drawer_name, drawer_state in state["children_states"].items():
                drawer = self._get_drawer_by_name(drawer_name)
                if drawer is not None:
                    drawer.load_state(drawer_state)
        if self.only_reduced_objects:
            self.settings_widget.reduced_only_check_box.setChecked(True)

        self.settings_widget.load_state(state)

    def clear(self):
        self.all_objects = None
        self.reduced_objects = None
        super().clear()

    def append_legend_part(self, legend_df, legend_dict):
        if self.is_enabled:
            drawer_name = self.name
            legend_dict["drawer_name"] = [drawer_name]
            for drawer in self.drawers:
                legend_df = drawer.append_legend_part(legend_df, legend_dict)
        return legend_df