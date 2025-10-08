import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.gui.drawers.DrawerFilledPolygon import DrawerFilledPolygon


class DrawerCellOCG(DrawerFilledPolygon):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str):
        super().__init__(parent, name, plot_item, color)
        self.color = color
        self.graphic_object.mouseDoubleClickEvent = self.on_click

    def on_click(self, mouse_click_event):
        self.parent.on_click(self, mouse_click_event)

    def set_data(self, data: np.ndarray):
        cell_points = pd.DataFrame({"x_points": data[:, 2].astype(np.float32),
                                    "y_points": data[:, 3].astype(np.float32),
                                    "scan_index": data[:, 1].astype(int),
                                    "df_indexes": data[:, 0].astype(int)})

        cell_points["connect_mask"] = ~np.isnan(cell_points["x_points"])
        cell_points_grouped = cell_points.groupby(by="scan_index")

        self.x_data = cell_points_grouped["x_points"].apply(np.array).to_dict()
        self.y_data = cell_points_grouped["y_points"].apply(np.array).to_dict()
        self.is_visible = cell_points_grouped["connect_mask"].apply(np.array).to_dict()
        self.df_indexes = cell_points_grouped["df_indexes"].apply(np.array).to_dict()
