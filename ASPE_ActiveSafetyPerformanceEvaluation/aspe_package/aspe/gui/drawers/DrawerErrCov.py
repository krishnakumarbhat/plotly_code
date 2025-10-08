from copy import deepcopy
from typing import Union

import numpy as np
import pandas as pd
import pyqtgraph as pg

from aspe.extractors.F360.Enums.f360_objects import F360ObjectsFilterType
from aspe.gui.drawers.DrawerLines import DrawerLines
from aspe.gui.views.settings.ErrCovDrawerSettingsWidget import ErrCovDrawerSettingsWidget


def set_reduced_objects(extracted_data_set):
    try:
        reduced_ds = deepcopy(extracted_data_set)
        reduced_mask = reduced_ds.loc[:, "reduced_id"] != 0
        reduced_ds = extracted_data_set.loc[reduced_mask, :]
        return reduced_ds
    except KeyError:
        return extracted_data_set


class DrawerErrCov(DrawerLines):
    def __init__(self,
                 parent,
                 name: str,
                 plot_item: pg.PlotItem,
                 color: str,
                 line_style: str,
                 line_width: float,
                 data_set):
        super().__init__(parent, name, plot_item, color, line_style, line_width)
        self.disable()
        self.line_points = 40
        self.confidence_interval = 0.9545
        self.set_data(data_set)
        self.settings_widget = self.create_settings_widget()

    def set_data(self, data_df):
        self.data_df = self.ellipse_data_prepare(data_df, self.cov_data_indexes())
        ellipse_x, ellipse_y = self.calculate_points()

        scan_index_raw = self.data_df.loc[:, "scan_index"].to_numpy()
        df_indexes_raw = self.data_df.index.to_numpy()
        scan_index_repeated = np.tile(scan_index_raw, (self.line_points + 2, 1)).T.reshape(-1)
        df_index_repeated = np.tile(df_indexes_raw, (self.line_points + 2, 1)).T.reshape(-1)

        ellipse = pd.DataFrame({"ellipse_x": ellipse_x.astype(np.float32),
                                "ellipse_y": ellipse_y.astype(np.float32),
                                "scan_index": scan_index_repeated,
                                "df_indexes": df_index_repeated})
        nan_mask = np.isnan(ellipse.loc[:, "ellipse_x"].to_numpy())
        ellipse["connect_mask"] = ~(nan_mask | np.roll(nan_mask, 0))
        ellipse_grouped = ellipse.groupby(by="scan_index")
        self.x_data = ellipse_grouped["ellipse_x"].apply(np.array).to_dict()
        self.y_data = ellipse_grouped["ellipse_y"].apply(np.array).to_dict()
        self.is_visible = ellipse_grouped["connect_mask"].apply(np.array).to_dict()
        self.df_indexes = ellipse_grouped["df_indexes"].apply(np.array).to_dict()

    def ellipse_data_prepare(self, data_df, sig_indexes):
        ctca = data_df.filter_type == F360ObjectsFilterType.CTCA
        ccv = data_df.filter_type == F360ObjectsFilterType.CCV
        new_data_df = data_df.copy()

        new_data_df.loc[ctca, "var_x"] = data_df.loc[ctca, "motion_model_variances_suppl"].str[sig_indexes[0]]
        new_data_df.loc[ctca, "var_y"] = data_df.loc[ctca, "motion_model_variances_suppl"].str[sig_indexes[1]]
        new_data_df.loc[ctca, "cov_xy"] = data_df.loc[ctca, "motion_model_covariances_suppl"].str[sig_indexes[2]]

        new_data_df.loc[ccv, "var_x"] = data_df.loc[ccv, "motion_model_variances_suppl"].str[sig_indexes[3]]
        new_data_df.loc[ccv, "var_y"] = data_df.loc[ccv, "motion_model_variances_suppl"].str[sig_indexes[4]]
        new_data_df.loc[ccv, "cov_xy"] = data_df.loc[ccv, "motion_model_covariances_suppl"].str[sig_indexes[5]]

        return new_data_df

    @staticmethod
    def _change_series_to_numpy_representation(array: Union[pd.Series, np.ndarray]):
        if isinstance(array, pd.Series):
            array = array.to_numpy()
        return array

    def calc_cov_ellipse_points(self,
                                variances_x: Union[pd.Series, np.ndarray],
                                variances_y: Union[pd.Series, np.ndarray],
                                covariances: Union[pd.Series, np.ndarray],
                                line_points: int,
                                confidence_interval):

        variances_x = self._change_series_to_numpy_representation(variances_x)
        variances_y = self._change_series_to_numpy_representation(variances_y)
        covariances = self._change_series_to_numpy_representation(covariances)

        # Unsafe casting to enable proper management of Nan, otherwise np.isnan crashes
        variances_x = variances_x.astype(float, copy=True)
        variances_y = variances_y.astype(float, copy=True)
        covariances = covariances.astype(float, copy=True)

        # Remove NaN
        variances_x[np.isnan(variances_x)] = 0
        variances_y[np.isnan(variances_y)] = 0
        covariances[np.isnan(covariances)] = 0

        # Ellipse points calculating
        ellipse_len = len(covariances)
        s = -2 * np.log(1 - confidence_interval)
        t = np.linspace(0, 2 * np.pi, line_points)

        circle_points = np.stack(ellipse_len * [np.array([np.sin(t), np.cos(t)])])

        sigma_row_0 = np.dstack([variances_x.reshape((-1, 1, 1)), covariances.reshape((-1, 1, 1))])
        sigma_row_1 = np.dstack([covariances.reshape((-1, 1, 1)), variances_y.reshape((-1, 1, 1))])
        sigma = np.hstack([sigma_row_0, sigma_row_1])
        eigenvalues, eigenvectors = np.linalg.eig(s * sigma)

        eigenvalues_diag_row_0 = np.dstack([eigenvalues[:, 0].reshape(-1, 1, 1), np.zeros([ellipse_len, 1, 1])])
        eigenvalues_diag_row_1 = np.dstack([np.zeros([ellipse_len, 1, 1]), eigenvalues[:, 1].reshape(-1, 1, 1)])
        eigenvalues_diag = np.hstack([eigenvalues_diag_row_0, eigenvalues_diag_row_1])
        return eigenvectors @ (np.sqrt(np.real(eigenvalues_diag)) @ circle_points)

    def move_ellipse_to_center(self, ellipse_points):
        pass

    def ellipse_data_process_to_display(self, ellipse_points):
        ellipse_len = len(ellipse_points[:, 0, 0].reshape(-1))
        nan_column = np.full((ellipse_len, 2, 1), np.nan)
        first_column_to_repeat = ellipse_points[:, :, 0].reshape(-1, 2, 1)
        ellipse_points = np.dstack([ellipse_points, first_column_to_repeat, nan_column])
        ellipse_x = ellipse_points[:, 0, :].reshape(-1)
        ellipse_y = ellipse_points[:, 1, :].reshape(-1)

        return ellipse_x, ellipse_y

    def create_settings_widget(self):
        self.settings_widget = ErrCovDrawerSettingsWidget(self.name, self)
        self.update_setting_widget()
        return self.settings_widget

    def change_confidence_interval(self, new_confidence_interval):
        self.confidence_interval = new_confidence_interval
        self.set_data(self.data_df)
        self.plot_scan_index(self.parent.current_scan_index)

    def get_state(self):
        return {
            "is_enabled": self.is_enabled,
            "color": self.color,
            "line_style": self.line_style,
            "line_width": self.line_width,
            "confidence": self.confidence_interval,
        }

    def load_state(self, state):
        self.is_enabled = state["is_enabled"]
        self.color = state["color"]
        self.line_style = state["line_style"]
        self.line_width = state["line_width"]
        self.confidence_interval = state["confidence"]
        self.settings_widget.load_state(state)
        self.update_graphic_object_style()
        if not self.is_enabled:
            self.disable()

    def update_setting_widget(self):
        state = self.get_state()
        self.settings_widget.load_state(state)
