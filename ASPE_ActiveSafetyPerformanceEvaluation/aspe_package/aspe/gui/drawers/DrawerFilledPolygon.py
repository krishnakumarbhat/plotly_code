import numpy as np
import pyqtgraph as pg
from PyQt5.QtWidgets import QGraphicsPathItem

from aspe.gui.drawers.abstract_drawers.DrawerLeaf import DrawerLeaf
from aspe.gui.views.settings.FilledPolygonDrawerSettingsWidget import FilledPolygonDrawerSettingsWidget


class DrawerFilledPolygon(DrawerLeaf):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str):
        super().__init__(parent, name, plot_item, color)
        self.graphic_object = QGraphicsPathItem()
        self.graphic_object.setPen(pg.mkPen(color=color))
        self.graphic_object.setBrush(pg.mkBrush(color=self.color))
        self.plot_item.addItem(self.graphic_object)

    def plot_scan_index(self, scan_index):
        if self.is_enabled:
            try:
                x = self.x_data[scan_index]
                y = self.y_data[scan_index]
                visible_mask = self.is_visible[scan_index]
                nan_mask = np.isnan(x)
                x[nan_mask] = np.nan
                y[nan_mask] = np.nan
                path = pg.arrayToQPath(y, x, connect=visible_mask)
                self.graphic_object.setPath(path)
            except KeyError:
                self.plot_empty_data()

    def plot_empty_data(self):
        self.graphic_object.setPath(pg.arrayToQPath(np.array([]), np.array([])))

    def update_graphic_object_style(self):
        brush = pg.mkBrush(color=self.color)
        self.graphic_object.setBrush(brush)

    def get_state(self):
        return {
            "is_enabled": self.is_enabled,
            "color": self.color,
        }

    def create_settings_widget(self):
        self.settings_widget = FilledPolygonDrawerSettingsWidget(self.name, self)
        return self.settings_widget

    def load_state(self, state):
        self.is_enabled = state["is_enabled"]
        self.color = state["color"]
        self.settings_widget.load_state(state)
        self.update_graphic_object_style()
        if not self.is_enabled:
            self.disable()
