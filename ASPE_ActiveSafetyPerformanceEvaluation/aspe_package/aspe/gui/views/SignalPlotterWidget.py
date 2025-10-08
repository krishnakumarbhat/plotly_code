from enum import Enum

import numpy as np
import pandas as pd
import pyqtgraph as pg
from PyQt5 import QtCore, QtGui, QtWidgets

COLOR_PALETTE = [
    "#1f77b4",
    "#ff7f0e",
    "#5FD35F",
    "#d62728",
    "#9467bd",
    "#bcbd22",
    "#7f7f7f",
    "#17becf",
    "#8c564b",
    "#008000",
    "#800000",
    "#FF00FF",
    "#FFFF00",
    "#000000",
     ]

class SignalPlotterWidget(QtWidgets.QWidget):
    def __init__(self, controller, *args, **kwargs):
        super().__init__()
        self.controller = controller
        self.setupUi()
        self.graphic_win = pg.GraphicsWindow(parent=self)
        self.graphic_win.setBackground(pg.mkBrush(color="#ffffff"))
        self.axes_vlayout.addWidget(self.graphic_win)
        self.axes = []
        self.scan_index_markers = []
        self.lines_count = 0
        self.default_lines_width = 2
        self.plotted_signals = pd.DataFrame()
        self.connect_signals()

    def setupUi(self):  # noqa: PLR0915
        self.setObjectName("SignalPloterWidget")
        self.resize(1400, 400)
        self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint)
        self.horizontalLayout = QtWidgets.QHBoxLayout(self)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.axes_widget = QtWidgets.QWidget(self)
        self.axes_widget.setObjectName("axes_widget")
        self.axes_vlayout = QtWidgets.QVBoxLayout(self.axes_widget)
        self.axes_vlayout.setObjectName("axes_vlayout")
        self.horizontalLayout.addWidget(self.axes_widget)
        self.right_panel_widget = QtWidgets.QWidget(self)
        self.right_panel_widget.setObjectName("right_panel_widget")
        self.righ_panel_vlayout = QtWidgets.QVBoxLayout(self.right_panel_widget)
        self.righ_panel_vlayout.setObjectName("righ_panel_vlayout")
        self.signal_info_table = QtWidgets.QTableWidget(self.right_panel_widget)
        self.signal_info_table.setObjectName("signal_info_table")
        self.signal_info_table.setColumnCount(5)
        self.signal_info_table.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        self.signal_info_table.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        self.signal_info_table.setHorizontalHeaderItem(1, item)
        item = QtWidgets.QTableWidgetItem()
        self.signal_info_table.setHorizontalHeaderItem(2, item)
        item = QtWidgets.QTableWidgetItem()
        self.signal_info_table.setHorizontalHeaderItem(3, item)
        item = QtWidgets.QTableWidgetItem()
        self.signal_info_table.setHorizontalHeaderItem(4, item)
        self.signal_info_table.horizontalHeader().setDefaultSectionSize(100)
        self.signal_info_table.horizontalHeader().setMinimumSectionSize(10)
        self.signal_info_table.horizontalHeader().setStretchLastSection(True)
        self.righ_panel_vlayout.addWidget(self.signal_info_table)
        self.horizontalLayout.addWidget(self.right_panel_widget)
        self.horizontalLayout.setStretch(0, 8)
        self.horizontalLayout.setStretch(1, 6)

        self.signal_plotter_control_buttons_widget = QtWidgets.QWidget(self)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Minimum)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.signal_plotter_control_buttons_widget.sizePolicy().hasHeightForWidth())
        self.signal_plotter_control_buttons_widget.setSizePolicy(sizePolicy)
        self.signal_plotter_control_buttons_widget.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.signal_plotter_control_buttons_widget.setObjectName("signal_plotter_control_buttons_widget")

        self.horizontalButtonsLayout = QtWidgets.QHBoxLayout(self.signal_plotter_control_buttons_widget)
        self.horizontalButtonsLayout.setObjectName("horizontalButtonsLayout")

        self.signal_plotter_control_buttons_layout = QtWidgets.QHBoxLayout()
        self.signal_plotter_control_buttons_layout.setObjectName("signal_plotter_control_buttons_layout")

        self.remove_row_button = QtWidgets.QPushButton(self.signal_plotter_control_buttons_widget)
        self.remove_row_button.setCheckable(True)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Maximum)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setHeightForWidth(self.remove_row_button.sizePolicy().hasHeightForWidth())
        self.remove_row_button.setSizePolicy(sizePolicy)
        self.remove_row_button.setMaximumSize(QtCore.QSize(64, 32))
        self.remove_row_button.setObjectName("remove_row_button")
        self.signal_plotter_control_buttons_layout.addWidget(self.remove_row_button)

        self.signal_plotter_control_buttons_layout.setStretch(4, 1)
        self.signal_plotter_control_buttons_layout.setStretch(5, 1)
        self.righ_panel_vlayout.addLayout(self.signal_plotter_control_buttons_layout)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.remove_row_button.setText(_translate("PlaybackPanelWidget", "Remove"))
        self.setWindowTitle("Signal Ploter")
        item = self.signal_info_table.horizontalHeaderItem(0)
        item.setText("Source")
        item = self.signal_info_table.horizontalHeaderItem(1)
        item.setText("Data set")
        item = self.signal_info_table.horizontalHeaderItem(2)
        item.setText("Signal signature")
        item = self.signal_info_table.horizontalHeaderItem(3)
        item.setText("Unique ID")
        item = self.signal_info_table.horizontalHeaderItem(4)
        item.setText("Value")

    def create_new_plot(self):
        current_axes_count = len(self.axes)
        ax = self.graphic_win.addPlot(row=current_axes_count, col=0)
        ax.showGrid(x=True, y=True, alpha=0.5)
        if len(self.axes) > 0:
            ax.setXLink(self.axes[0])
        infinity_line = pg.InfiniteLine(pen=pg.mkPen(color="#ff0000", width=self.default_lines_width))
        ax.addItem(infinity_line)
        self.axes.append(ax)
        self.scan_index_markers.append(infinity_line)

    def plot_signal(self, source_signature, data_set_signature, selected_data, signal_to_plot_signature, axes_index):
        sorted_data = selected_data.sort_values(by="scan_index")
        x_signal = sorted_data.loc[:, "scan_index"].to_numpy().astype(int).copy()
        y_signal = sorted_data.loc[:, signal_to_plot_signature].to_numpy().copy()
        # first element, because every array element has the same value
        unique_id = sorted_data.loc[:, "unique_id"].to_numpy()[0]

        line_color = COLOR_PALETTE[self.lines_count % len(COLOR_PALETTE)]
        line_pen = pg.mkPen(color=line_color, width=self.default_lines_width)

        x_dif = np.diff(x_signal)
        x_jumps = np.append((x_dif != 1), [0])  # append cause diff returns n-1 shape

        if isinstance(y_signal[0], Enum):
            y_signal_len = len(y_signal)
            for idx in range(y_signal_len):
                y_signal[idx] = y_signal[idx].value
            y_signal = y_signal.astype(np.int32)
        nan_mask = np.isnan(y_signal.astype("float32"))
        y_signal[nan_mask] = 0.0

        plot_item = self.axes[axes_index].plot(x=x_signal, y=y_signal, pen=line_pen, connect=~nan_mask & ~x_jumps)
        current_plotted_signal = pd.DataFrame({"scan_index": x_signal,
                                               "signal_source": source_signature,
                                               "signal_data_set": data_set_signature,
                                               "signal_name": signal_to_plot_signature,
                                               "unique_id": unique_id,
                                               "signal_value": y_signal,
                                               "line_color": line_color,
                                               "plot_item": plot_item,
                                               "line_number": self.lines_count,
                                               }).convert_dtypes()
        self.plotted_signals = self.plotted_signals.append(current_plotted_signal)
        self.add_signal_legend_to_table(source_signature,
                                        data_set_signature,
                                        signal_to_plot_signature,
                                        line_color,
                                        unique_id)
        self.lines_count += 1

    def update_signal_plotter(self, scan_index):
        self.update_scan_index_markers(scan_index)
        self.update_scan_index_signal_value(scan_index)

    def update_scan_index_markers(self, scan_index):
        for inf_line in self.scan_index_markers:
            inf_line.setValue(scan_index)

    def update_scan_index_signal_value(self, scan_index):
        for signal_index in range(self.signal_info_table.rowCount()):
            current_signal = self.plotted_signals.loc[self.plotted_signals["scan_index"] == scan_index]
            current_signal = current_signal.loc[current_signal["line_number"] == signal_index]
            current_signal = current_signal.set_index("scan_index")
            if not current_signal.empty:
                current_value = current_signal.loc[scan_index, "signal_value"]
                line_color = current_signal.loc[scan_index, "line_color"]
                # take object name if signal is Enum
                if isinstance(current_value, Enum):
                    current_value = current_value.name
            else:
                current_value = np.nan
                line_color = "#ffffff" #display as white word to disapear in the background
            # display signal current value
            item_value = QtGui.QTableWidgetItem(str(current_value))
            item_value.setForeground(pg.mkBrush(color=line_color))
            self.signal_info_table.setItem(signal_index, 4, item_value)

    def add_signal_legend_to_table(self,
                                   source_signature,
                                   data_set_signature,
                                   signal_to_plot_signature,
                                   line_color,
                                   unique_id):
        row_position = self.signal_info_table.rowCount()
        self.signal_info_table.insertRow(row_position)

        item_source = QtGui.QTableWidgetItem(source_signature)
        item_data_set = QtGui.QTableWidgetItem(data_set_signature)
        item_signal = QtGui.QTableWidgetItem(signal_to_plot_signature)
        item_unique_id = QtGui.QTableWidgetItem(str(unique_id))

        item_source.setForeground(pg.mkBrush(color=line_color))
        item_signal.setForeground(pg.mkBrush(color=line_color))
        item_data_set.setForeground(pg.mkBrush(color=line_color))
        item_unique_id.setForeground(pg.mkBrush(color=line_color))

        self.signal_info_table.setItem(row_position, 0, item_source)
        self.signal_info_table.setItem(row_position, 1, item_data_set)
        self.signal_info_table.setItem(row_position, 2, item_signal)
        self.signal_info_table.setItem(row_position, 3, item_unique_id)

    def closeEvent(self, close_event):
        self.clear()
        self.close()
        self.controller.on_signal_plotter_widget_close()

    def clear(self):
        self.graphic_win.close()
        self.axes = []
        self.scan_index_markers = []
        self.lines_count = 0
        self.plotted_signals = None

    def connect_signals(self):
        self.remove_row_button.pressed.connect(self.remove_selected_row)

    def remove_selected_row(self):
        if self.signal_info_table.selectedItems():
            selected_signal_number = self.signal_info_table.currentRow()
            signal_to_remove = \
                self.plotted_signals[self.plotted_signals["line_number"] ==
                                     selected_signal_number]["plot_item"].drop_duplicates().to_list().pop()
            # remove signal data
            self.signal_info_table.removeRow(selected_signal_number)
            self.plotted_signals = \
                self.plotted_signals[self.plotted_signals["plot_item"] !=
                                     signal_to_remove].reset_index().drop(columns="index")
            self.shift_signal_row_numbers()
            # remove signal curve
            for idx, axes in enumerate(self.axes):
                if signal_to_remove in axes.items:
                    axes.removeItem(signal_to_remove)
                    signal_axes_to_remove = idx
                    break
            if not self.axes[signal_axes_to_remove].curves:
                self.graphic_win.removeItem(self.axes[signal_axes_to_remove])
                self.axes.pop(signal_axes_to_remove)
                self.scan_index_markers.pop(signal_axes_to_remove)
            self.lines_count = self.lines_count - 1

    def shift_signal_row_numbers(self):
        signal_numbers = self.plotted_signals["line_number"].drop_duplicates().to_list()
        if signal_numbers:
            idx_prev = -1
            for idx in signal_numbers:
                if idx == 0:
                    idx_prev = idx
                else:
                    if idx - idx_prev != 1:
                        break
                    idx_prev = idx
            shift_signal_numbers = signal_numbers[signal_numbers.index(idx)::]
            for sig_number_to_shift in shift_signal_numbers:
                shift_indexes = np.where(self.plotted_signals["line_number"] == sig_number_to_shift)[0]
                self.plotted_signals.loc[shift_indexes, "line_number"] = sig_number_to_shift - 1


