import pandas as pd
import pyqtgraph as pg
from PyQt5 import QtGui, QtWidgets
from PyQt5.QtWidgets import QMenu, QSizePolicy, QTableWidget


class DataSourcesTable(QTableWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.parent_widget = None

        self.load_data_panel_controller = None
        self.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.loaded_color = "#ff0000"
        self.setSizeAdjustPolicy(QtWidgets.QAbstractScrollArea.AdjustToContents)
        self.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)
        self.setColumnCount(2)
        self.setObjectName("data_sources_table")
        self.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        self.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        self.setHorizontalHeaderItem(1, item)
        item = QtWidgets.QTableWidgetItem()
        self.setHorizontalHeaderItem(2, item)
        self.horizontalHeader().setCascadingSectionResizes(False)
        self.horizontalHeader().setDefaultSectionSize(300)
        self.horizontalHeader().setStretchLastSection(True)
        self.setSizePolicy(QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Maximum))
        self.setMaximumHeight(100)

        self.setSortingEnabled(True)

        item = QtWidgets.QTableWidgetItem()
        item.setText("subfolder")
        self.setHorizontalHeaderItem(0, item)

        item = QtWidgets.QTableWidgetItem()
        item.setText("type")
        self.setHorizontalHeaderItem(1, item)

    def add_row(self, data_source: pd.Series):
        row_position = self.rowCount()
        self.insertRow(row_position)
        self.setItem(row_position, 0, QtGui.QTableWidgetItem(data_source.root_folder))
        self.setItem(row_position, 1, QtGui.QTableWidgetItem(data_source.type))

    def contextMenuEvent(self, event):
        menu = QMenu(self)
        load_action = menu.addAction("Load selected")
        clear_action = menu.addAction("Clear selected")
        self.selectedIndexes()
        action = menu.exec_(self.mapToGlobal(event.pos()))
        if action == load_action:
            self.on_load_clicked()
        elif action == clear_action:
            self.on_clear_clicked()

    def on_load_clicked(self):
        selected_rows = self.selectionModel().selectedRows()
        selected_rows_idxs = [r.row() for r in selected_rows]
        for row_idx in selected_rows_idxs:
            source_info_dict = {
                "root_folder": self.item(row_idx, 0).text(),
                "type": self.item(row_idx, 1).text(),
            }
            f_loaded = self.parent_widget.on_load_data_from_source(source_info_dict)
            print("f_loaded: ", f_loaded)
            if f_loaded:
                for n in range(0, self.columnCount()):
                    item = self.item(row_idx, n)
                    item.setForeground(pg.mkBrush(color=self.loaded_color))
        self.clearSelection()

    def on_clear_clicked(self):
        selected_rows = self.selectionModel().selectedRows()
        selected_rows_idxs = [r.row() for r in selected_rows]
        for row_idx in selected_rows_idxs:
            selected_item = self.item(row_idx, 0)
            if selected_item.foreground().color().name() == self.loaded_color:
                source_info_dict = {
                    "root_folder": self.item(row_idx, 0).text(),
                    "type": self.item(row_idx, 1).text(),
                }
                self.parent_widget.clear_data_using_source_info_dict(source_info_dict)
                for n in range(0, self.columnCount()):
                    item = self.item(row_idx, n)
                    item.setForeground(pg.mkBrush(color="#000000"))
        self.clearSelection()

    def clear_sources(self):
        self.setRowCount(0)
        self.available_sources_df = {}
