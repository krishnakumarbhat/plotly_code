import contextlib

import pyqtgraph as pg

from aspe.gui.drawers.abstract_drawers.DrawerComponent import DrawerComponent


class DrawerLeaf(DrawerComponent):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str):
        """
        @param parent: parent drawer
        @param name: name of the drawer
        @param plot_item: object of pg.PlotItem (canvas)
        @param color: colors are set in HEX format as '#RRGGBBAA' where R stands for red value,
        G stands for green value, B stands for blue value and A stands for Alpha value.
        Using alpha value is not necessary, it is possible to
        use '#RRGGBB' color format -  in this case alpha is set to max.RF
        """
        super().__init__(parent, name)
        self.color = color
        self.plot_item = plot_item
        self.x_data = {}
        self.y_data = {}
        self.df_indexes = {}
        self.is_visible = {}
        self.graphic_object = None  # reference to pyqtgraph obj representing plot - line or points
        self.settings_widget = None

    def enable(self):
        self.is_enabled = True
        self.plot_scan_index(self.parent.current_scan_index)

    def disable(self):
        self.is_enabled = False
        self.plot_empty_data()

    def change_line_color(self, color):
        self.color = color
        self.update_graphic_object_style()

    def on_click(self, mouse_click_event):
        self.parent.on_click(self, mouse_click_event)

    def plot_empty_data(self):
        self.graphic_object.setData([], [])

    def clear(self):
        self.plot_item.removeItem(self.graphic_object)
        self.plot_empty_data()
        self.graphic_object = None
        if self.settings_widget is not None:
            self.settings_widget.remove()
            self.settings_widget = None
        self.x_data = {}
        self.y_data = {}
        self.df_indexes = {}
        self.parent = None
        self.plot_item = None
        self.df_indexes = None
        self.is_visible = None
        self.data_df = None
        with contextlib.suppress(KeyError):
            self.labels_text = {}


    def append_legend_part(self, legend_df, legend_dict):
        return legend_df
