import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.gui.drawers.DrawerLines import DrawerLines


class Drawer2ndDegreePolynomial(DrawerLines):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str, line_style: str, line_width: float):
        super().__init__(parent, name, plot_item, color, line_style, line_width)

    def set_data(self, data_df: pd.DataFrame, p2_signature: str, p1_signature: str, p0_signature: str,
                 min_x_signature: str, max_x_signature: str):
        """
        :param data_df: pd.DataFrame with polynomials data
        :param p2_signature: signature of column which contains 2nd degree coefficient
        :param p1_signature: signature of column which contains 1st degree coefficient
        :param p0_signature: signature of column which contains 0 degree coefficient
        :param min_x_signature: signature of column which contain min x value for polynomial
        :param max_x_signature: signature of column which contain max x value for polynomial
        :return:
        """

        # First calculate polynomials value for all scans at single step for efficiency. For now assume that all
        # polynomials are in some large range - later on out of bounds samples will be filtered out
        p2 = data_df.loc[:, p2_signature].to_numpy().reshape((-1, 1))
        p1 = data_df.loc[:, p1_signature].to_numpy().reshape((-1, 1))
        p0 = data_df.loc[:, p0_signature].to_numpy().reshape((-1, 1))

        poly_x = np.arange(start=-200, stop=200, step=0.5)
        poly_x = np.tile(poly_x, reps=(len(data_df), 1))
        poly_y = p2 * (poly_x * poly_x) + p1 * poly_x + p0

        # Split calculated samples to separated scan indexes
        for scan_index, scan_df in data_df.reset_index().groupby(by="scan_index"):
            min_x = scan_df.loc[:, min_x_signature].to_numpy().reshape((-1, 1))
            max_x = scan_df.loc[:, max_x_signature].to_numpy().reshape((-1, 1))
            df_indexes = scan_df.index.to_numpy().reshape((-1, 1))

            poly_x_scan = poly_x[scan_df.index.to_numpy()]
            poly_y_scan = poly_y[scan_df.index.to_numpy()]
            within_range = (min_x < poly_x_scan) & (poly_x_scan < max_x)
            df_indexes = np.tile(df_indexes, (1, poly_x.shape[1]))

            nan_column = np.full(shape=(poly_x_scan.shape[0], 1), fill_value=np.nan)
            true_column = np.full(shape=(poly_x_scan.shape[0], 1), fill_value=True)
            poly_x_scan = np.hstack([poly_x_scan, nan_column]).reshape(-1)
            poly_y_scan = np.hstack([poly_y_scan, nan_column]).reshape(-1)
            within_range = np.hstack([within_range, true_column]).reshape(-1)
            df_indexes = np.hstack([df_indexes, nan_column]).reshape(-1)

            # Filter out samples that are not in min/max polynomial range
            poly_x_scan = poly_x_scan[within_range]
            poly_y_scan = poly_y_scan[within_range]
            df_indexes = df_indexes[within_range]

            # Append boundary values - min/max
            lines_last_elements_indexes = np.where(np.isnan(poly_x_scan))[0]
            lines_number = len(lines_last_elements_indexes)
            if lines_number == 1:
                lines_first_elements_indexes = np.array([0])
            else:
                lines_first_elements_indexes = np.insert(lines_last_elements_indexes[0:-1] + 1, 0, 0, axis=0)
            insert_indexes = np.concatenate((lines_first_elements_indexes, lines_last_elements_indexes))

            min_x = min_x.reshape((1, -1))
            max_x = max_x.reshape((1, -1))
            insert_values_x = np.concatenate((min_x[0], max_x[0]))

            p2 = scan_df.loc[:, p2_signature].to_numpy().reshape((1, -1))
            p1 = scan_df.loc[:, p1_signature].to_numpy().reshape((1, -1))
            p0 = scan_df.loc[:, p0_signature].to_numpy().reshape((1, -1))

            poly_y_scan_min = p2 * (min_x * min_x) + p1 * min_x + p0
            poly_y_scan_max = p2 * (max_x * max_x) + p1 * max_x + p0
            insert_values_y = np.concatenate((poly_y_scan_min[0], poly_y_scan_max[0]))

            insert_values_idx = np.array(2 * [df_indexes[lines_last_elements_indexes - 1]]).reshape(1, -1)

            poly_x_scan = np.insert(poly_x_scan, insert_indexes, insert_values_x, axis=0)
            poly_y_scan = np.insert(poly_y_scan, insert_indexes, insert_values_y, axis=0)
            df_indexes = np.insert(df_indexes, insert_indexes, insert_values_idx, axis=0)

            # Create connect mask to split separate polynomials
            nan_mask = np.isnan(poly_x_scan)
            connect_mask = ~(nan_mask | np.roll(nan_mask, -1))

            self.x_data[scan_index] = poly_x_scan
            self.y_data[scan_index] = poly_y_scan
            self.is_visible[scan_index] = connect_mask
            self.df_indexes[scan_index] = df_indexes