from itertools import product
from typing import Tuple

import numpy as np
import pandas as pd
from shapely.geometry import Point, Polygon

from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.DrawerBBoxes import DrawerBBoxes
from aspe.gui.drawers.DrawerCellOCG import DrawerCellOCG
from aspe.gui.utilities.calc_occupancy_grid import calc_occupancy_grid


def map_to_color(state):
    colors_palette = {
        0: "#ff0000",  # red STATUS_CAN_NOT_PASS_UNDER
        1: "#ffff00",  # yellow STATUS_IS_LIKELY_TO_PASS_UNDER
        2: "#00ff00",  # green STATUS_CAN_PASS_UNDER
        3: "#32A8A8",  # light blue STATUS_NOT_TO_CONSIDER
    }

    return colors_palette.get(state, "#000000")


def map_to_alpha(confidence):
    alpha_palette = {
        0: "33",  # equals confidence 0.2 or lower
        1: "66",  # equals confidence (0.2;0.4>
        2: "99",  # equals confidence (0.4;0.6>
        3: "cc",  # equals confidence (0.6;0.8>
        4: "ff",  # equals confidence above 0.8
    }

    return alpha_palette.get(confidence, "00")


class DrawerOccupancyGrid(DrawerComposite):
    """
    Class to handle occupancy grid visualization. It calculates grid points and adds cell drawers.
    Drawers are added for every drivability state and classification confidence level.
    """

    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)

    def create_drawers(self, plot_item) -> list:
        data_df = self.data_set.per_scan_index
        data_df = data_df.reset_index()  # make sure indexes pair with number of rows

        data_df["x_path"], data_df["y_path"] = \
            zip(*data_df.apply(lambda row: calc_occupancy_grid(x_tick=row["cell_length"],
                                                               y_tick=row["cell_width"],
                                                               x_offset=row["x_offset"],
                                                               y_offset=row["y_offset"],
                                                               x_n_cells_close=row["x_n_cells_close"].astype(int),
                                                               x_n_cells_mid=row["x_n_cells_mid"].astype(int),
                                                               x_n_cells_far=row["x_n_cells_far"].astype(int),
                                                               y_n_cells=row["y_n_cells"].astype(int),
                                                               factor=row["width_extension_factor"],
                                                               angle=row["grid_orientation"],
                                                               curvature=row["curvature"]), axis=1))
        self.data_set.signals["x_path"] = data_df["x_path"]
        self.data_set.signals["y_path"] = data_df["y_path"]

        grid_df = self.data_set.signals
        grid_df['confidence_level'] = 2  # TODO FZE-400 hotfix confidence not present in extracted/parsed data
        extracted_paths = self.extract_cell_polygon_paths(data_df, grid_df)
        drawers = []

        # iterate through all possible products
        confidence_unique = np.unique(extracted_paths[:, :, 4])
        state_unique = np.unique(extracted_paths[:, :, 5])
        for ix_state, ix_confidence in product(confidence_unique, state_unique):
            color = map_to_color(ix_state) + map_to_alpha(ix_confidence)
            drawer = DrawerCellOCG(self, f"cell {ix_state}, {ix_confidence}", plot_item, color=color)
            # apply mask, to choose only matching data
            drawer.set_data(extracted_paths[np.logical_and((extracted_paths[:, :, 4] == ix_state),
                                                           (extracted_paths[:, :, 5] == ix_confidence))])
            drawers.append(drawer)
        return drawers

    def create_selection_drawer(self, plot_item) -> DrawerBBoxes:
        return DrawerBBoxes(self, "PolyBoxSelect", plot_item, color="#0000FF", line_style="--", line_width=2)

    def on_click(self, clicked_drawer: DrawerCellOCG, click_event) -> None:
        if clicked_drawer is self.selection_drawer:
            self.deselect()
        else:
            # Get the click position
            clicked_point = click_event.pos()
            # Get polygons point for current scan
            n_poly_pts = 6
            point = Point(clicked_point.y(), clicked_point.x())
            for drawer in self.drawers:
                # Check if data for current index exists for a drawer for plotting cells.
                # Scan indexes are stored as keys inside the drawers
                x_current_scan = drawer.x_data.get(self.current_scan_index, None)
                y_current_scan = drawer.y_data.get(self.current_scan_index, None)

                if (isinstance(x_current_scan, type(None)) and isinstance(y_current_scan, type(None))) is False:
                    n_cells = int(len(x_current_scan) / n_poly_pts)
                    # Create polygons out of these pts and find which polygon belongs to the clicked event
                    x_current_scan = np.reshape(x_current_scan, (n_cells, n_poly_pts))
                    y_current_scan = np.reshape(y_current_scan, (n_cells, n_poly_pts))
                    poly_centroid, length, width, cell_id = \
                        self.get_selection_polygon_params(point, x_current_scan, y_current_scan)

                    n_polygons = int(len(self.data_set.signals["x_path"][self.current_scan_index]) / n_poly_pts)
                    current_poly_list_x = \
                        np.reshape(self.data_set.signals["x_path"][self.current_scan_index], (n_polygons, n_poly_pts))
                    current_poly_list_y = \
                        np.reshape(self.data_set.signals["y_path"][self.current_scan_index], (n_polygons, n_poly_pts))
                    # Get data frame index , corresponding to selected OCG cell
                    if (poly_centroid and length and width and cell_id) is not None:
                        for i, (x, y) in enumerate(zip(current_poly_list_x, current_poly_list_y)):
                            if (np.isclose(x[:-1], x_current_scan[cell_id][:-1], rtol=1e-6)).all() and \
                                    (np.isclose(y[:-1], y_current_scan[cell_id][:-1], rtol=1e-6)).all():
                                cell_id = i
                                self.select(cell_id, n_poly_pts)
                                return
        return

    def get_selection_polygon_params(self,
                                     clicked_pt: Point,
                                     x_current_scan: np.ndarray,
                                     y_current_scan: np.ndarray) -> Tuple[Point, float, float, int]:
        for i, (x, y) in enumerate(zip(x_current_scan, y_current_scan)):
            coords = zip(x[:-1],y[:-1])
            poly = Polygon(coords)
            if poly.intersects(clicked_pt):
                centroid, length, width = self.get_polygon_params(poly)
                cell_id = i
                return centroid, length, width, cell_id
        return None, None, None, None

    def get_polygon_params(self, xy_coords: Polygon) -> Tuple[Point, float, float]:
        polygon = Polygon(xy_coords)
        # get minimum bounding box around polygon
        box = polygon.minimum_rotated_rectangle
        # get coordinates of polygon vertices
        x, y = box.exterior.coords.xy
        # get length of bounding box edges
        edge_length = (Point(x[0], y[0]).distance(Point(x[1], y[1])), Point(x[1], y[1]).distance(Point(x[2], y[2])))
        # get width of polygon as the longest edge of the bounding box
        width = max(edge_length)
        # get length of polygon as the shortest edge of the bounding box
        length = min(edge_length)
        return polygon.centroid, length, width

    def select(self, cell_id: int, n_polygon_points: int) -> None:
        if cell_id != self.selected_unique_id:
            self.selected_unique_id = cell_id
            # Initialize empty data frames
            self.data_set.per_scan_index.loc[:, "x_pos"] = 0.0
            self.data_set.per_scan_index.loc[:, "y_pos"] = 0.0
            self.data_set.per_scan_index.loc[:, "length"] = 0.0
            self.data_set.per_scan_index.loc[:, "width"] = 0.0
            self.data_set.per_scan_index.loc[:, "orientation"] = 0.0
            array_id = cell_id * n_polygon_points
            for scan_id in self.scan_indexes:

                x_coord = self.data_set.signals["x_path"][scan_id][array_id: array_id + n_polygon_points]
                y_coord = self.data_set.signals["y_path"][scan_id][array_id: array_id + n_polygon_points]
                xy_coords = zip(x_coord[:-1],y_coord[:-1])
                centroid, length, width = self.get_polygon_params(xy_coords)
                self.data_set.per_scan_index.loc[:, "x_pos"].at[scan_id] = centroid.x
                self.data_set.per_scan_index.loc[:, "y_pos"].at[scan_id] = centroid.y
                self.data_set.per_scan_index.loc[:, "length"].at[scan_id] = length
                self.data_set.per_scan_index.loc[:, "width"].at[scan_id] = width + 0.5

            selected_df = self.data_set.signals.loc[self.data_set.signals.loc[:, "unique_id"] == cell_id, :]
            if self.data_set.raw_signals is not None:
                raw_df = self.data_set.raw_signals.loc[self.data_set.raw_signals.loc[:, "unique_id"] == cell_id, :]
                selected_df = selected_df.join(raw_df, rsuffix="_raw")

            self.selected_data = selected_df
            self.selection_drawer.set_data(
                self.data_set.per_scan_index, "x_pos", "y_pos", "orientation", "length", "width")
            self.selection_drawer.plot_scan_index(self.current_scan_index)
            self.parent.on_select(self)
        else:
            self.deselect()

    @staticmethod
    def extract_cell_polygon_paths(data_df: pd.DataFrame, grid_df: pd.DataFrame) -> np.ndarray:
        """
        Based on dataframe containing log data create extracted_paths array to set drawers data.
        @param data_df: Dataframe containing log data and calculated occupancy grid data.
        @return: Stacked numpy array.
        Along axis 0 there is data related to subsequent cells.
        Along axis 1 there is data for subsequent scan indexes.
        Along axis 2 there are:
            - 0 - df indexes
            - 1 - scan indexes
            - 2 - x coordinates data
            - 3 - y coordinates data
            - 4 - cells states
            - 5 - confidences data
        """
        n_polygon_points = 6

        x_n_cells = data_df[["x_n_cells_close", "x_n_cells_mid", "x_n_cells_far"]].iloc[0].sum().astype(int)
        n_cells = x_n_cells * data_df["y_n_cells"].iloc[0].astype(int)
        # initialize empty arrays, based on grid shape
        x_result = np.zeros(shape=(n_cells, len(data_df) * n_polygon_points))
        y_result = np.zeros(shape=(n_cells, len(data_df) * n_polygon_points))
        confidence = np.zeros(shape=(n_cells, len(data_df) * n_polygon_points))
        state = np.zeros(shape=(n_cells, len(data_df) * n_polygon_points))
        scans = np.zeros(shape=(n_cells, len(data_df) * n_polygon_points))
        indexes = np.zeros(shape=(n_cells, len(data_df) * n_polygon_points))

        def put_data_into_array(array: np.ndarray, data: np.ndarray, ix: int) -> np.ndarray:
            array[:, ix * n_polygon_points:(ix + 1) * n_polygon_points] = data
            return array

        for ix in range(0, len(data_df)):
            # reshape x and y path, axis 0 - path, axis 1 - cells
            x_current_scan = np.reshape(data_df["x_path"].iloc[ix], (n_cells, n_polygon_points))
            x_result = put_data_into_array(x_result, x_current_scan, ix)

            y_current_scan = np.reshape(data_df["y_path"].iloc[ix], (n_cells, n_polygon_points))
            y_result = put_data_into_array(y_result, y_current_scan, ix)

            confidence_current_scan = grid_df["confidence_level"].iloc[ix * n_cells:(ix + 1) * n_cells]
            confidence = put_data_into_array(confidence, np.tile(confidence_current_scan, (n_polygon_points, 1)).T, ix)

            state_current_scan = grid_df["underdrivability_status"].iloc[ix * n_cells:(ix + 1) * n_cells]
            state = put_data_into_array(state, np.tile(state_current_scan, (n_polygon_points, 1)).T, ix)

            current_scan = data_df["scan_index"].iloc[ix]
            scans = put_data_into_array(scans, np.tile(current_scan, (n_cells, n_polygon_points)), ix)

            indexes = put_data_into_array(indexes, np.tile(ix, (n_cells, n_polygon_points)), ix)

        return np.dstack([indexes, scans, x_result, y_result, state, confidence])
