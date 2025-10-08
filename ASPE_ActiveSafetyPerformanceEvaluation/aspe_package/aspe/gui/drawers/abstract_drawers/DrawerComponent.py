from abc import ABC, abstractmethod


class DrawerComponent(ABC):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.is_enabled = True
        self.current_scan_index = None

    @abstractmethod
    def plot_scan_index(self, scan_index):
        pass

    @abstractmethod
    def plot_empty_data(self):
        pass

    @abstractmethod
    def enable(self):
        pass

    @abstractmethod
    def disable(self):
        pass

    @abstractmethod
    def on_click(self):
        pass

    @abstractmethod
    def clear(self):
        pass

    def append_legend_part(self, legend_df, legend_dict):
        return legend_df
