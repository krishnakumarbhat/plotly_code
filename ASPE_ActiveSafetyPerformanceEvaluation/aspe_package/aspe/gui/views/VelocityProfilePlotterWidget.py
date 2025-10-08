import numpy as np
import pyqtgraph as pg
from PyQt5 import QtCore, QtWidgets

SYMBOL_COLOR = [
    "#8bf7f7",
    "#e89be8",
    "#ffc975",
    "#26ff00"]


class VelocityProfilePlotterWidget(QtWidgets.QWidget):
    def __init__(self, controller, *args, **kwargs):
        super().__init__()
        self.controller = controller
        self.graphics_win_upper = pg.GraphicsWindow(parent=self)
        self.graphics_win_lower = pg.GraphicsWindow(parent=self)
        self.vlayout = QtWidgets.QVBoxLayout(self)

        self.setup_ui()

    def setup_ui(self):
        self.setObjectName("VelocityProfilePloterWidget")
        self.resize(1000, 600)
        self.setWindowTitle("VelocityProfilePlotter")
        self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint)

        self.vlayout.addWidget(self.graphics_win_upper, stretch=2)
        self.vlayout.addWidget(self.graphics_win_lower, stretch=1)

        self.graphics_win_upper.setBackground(pg.mkBrush(color="#ffffff"))
        self.graphics_win_lower.setBackground(pg.mkBrush(color="#ffffff"))

    def create_new_plot(self):
        self.ax_upper = self.graphics_win_upper.addPlot(title="Velocity profile  - not curvature compensated")
        self.ax_upper.showGrid(x=True, y=True, alpha=0.5)
        self.ax_upper.setLabel("left", "Range rate compensated [m/s]")
        self.ax_upper.setMenuEnabled(enableMenu=False)
        self.ax_upper.addLegend(offset=[-1, 1])
        self.ax_lower = self.graphics_win_lower.addPlot(title="Estimated range rate error - curvature compensated")
        self.ax_lower.showGrid(x=True, y=True, alpha=0.5)
        self.ax_lower.setLabel("left", "Estimated range rate error[m/s]")
        self.ax_lower.setLabel("bottom", "Azimuth VCS [deg]")
        self.ax_lower.setMenuEnabled(enableMenu=False)
        self.ax_lower.setXLink(self.ax_upper)

    def plot(self, associated_dets, not_associated_dets, selected_dets, est_azimuth_vec, est_range_rate_vec,
             est_range_rate_vec_refs=None):
        self.ax_upper.clear()
        self.ax_lower.clear()

        # Mark surrounding not associated detections for each sensor
        if len(not_associated_dets) != 0:
            for sensor_id, dets_single_sensor in not_associated_dets.groupby("sensor_id"):
                symbol_color = SYMBOL_COLOR[sensor_id % len(SYMBOL_COLOR)]
                self.ax_upper.plot(x=np.rad2deg(dets_single_sensor.loc[:, "azimuth_vcs_norm"].to_numpy()),
                                  y=dets_single_sensor.loc[:, "range_rate_comp"].to_numpy(),
                                  pen=None, symbolBrush=pg.mkBrush(color=symbol_color), symbol="d", symbolSize=12,
                                  name=f"sensor_id={sensor_id}")
                self.ax_lower.plot(x=np.rad2deg(dets_single_sensor.loc[:, "azimuth_vcs_norm"].to_numpy()),
                                  y=dets_single_sensor.loc[:, "est_range_rate_error"].to_numpy(),
                                  pen=None, symbolBrush=pg.mkBrush(color=symbol_color), symbol="d", symbolSize=12,
                                  name=f"sensor_id={sensor_id}")

        # Mark associated detections for each sensor
        if len(associated_dets) != 0:
            for sensor_id, dets_single_sensor in associated_dets.groupby("sensor_id"):
                symbol_color = SYMBOL_COLOR[sensor_id % len(SYMBOL_COLOR)]
                self.ax_upper.plot(x=np.rad2deg(dets_single_sensor.loc[:, "azimuth_vcs_norm"].to_numpy()),
                                  y=dets_single_sensor.loc[:, "range_rate_comp"].to_numpy(),
                                  pen=None, symbolBrush=pg.mkBrush(color=symbol_color), symbol="o", symbolSize=14,
                                  name=f"sensor_id={sensor_id} associated")
                self.ax_lower.plot(x=np.rad2deg(dets_single_sensor.loc[:, "azimuth_vcs_norm"].to_numpy()),
                                  y=dets_single_sensor.loc[:, "est_range_rate_error"].to_numpy(),
                                  pen=None, symbolBrush=pg.mkBrush(color=symbol_color), symbol="o", symbolSize=14,
                                  name=f"sensor_id={sensor_id} associated")

        # Mark detections selected_by_trk
        if len(selected_dets) != 0:
            self.ax_upper.plot(x=np.rad2deg(selected_dets.loc[:, "azimuth_vcs_norm"].to_numpy()),
                              y=selected_dets.loc[:, "range_rate_comp"].to_numpy(),
                              pen=None, symbolBrush="#ff0000", symbol="o", symbolSize=6, name="selected_by_trk")
            self.ax_lower.plot(x=np.rad2deg(selected_dets.loc[:, "azimuth_vcs_norm"].to_numpy()),
                              y=selected_dets.loc[:, "est_range_rate_error"].to_numpy(),
                              pen=None, symbolBrush="#ff0000", symbol="o", symbolSize=6, name="selected_by_trk")

        # Plot line based on estimated velocity
        if len(est_azimuth_vec) != 0:
            if est_range_rate_vec is not None:
                self.ax_upper.plot(x=np.rad2deg(est_azimuth_vec), y=est_range_rate_vec, pen="b", name="tracker object")
            if est_range_rate_vec_refs is not None:
                self.ax_upper.plot(x=np.rad2deg(est_azimuth_vec), y=est_range_rate_vec_refs, pen="r",
                                   name="ground truth")

    def clear(self):
        self.ax_upper.clear()
        self.ax_lower.clear()

    def closeEvent(self, close_event):
        self.graphics_win_upper.close()
        self.graphics_win_upper.close()
        self.close()
        self.controller.on_signal_plotter_widget_close()
