from enum import Enum

import numpy as np
from PyQt5 import QtCore, QtGui, QtWidgets

from aspe.gui.models.DataFrameTableModel import DataFrameTableModel
from aspe.gui.views.CollapsibleBoxWidget import CollapsibleBox


class DataViewerWidget(CollapsibleBox):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi()
        self.setContentLayout(self.widget_layout)
        self.data_model_to_widget = {}

    def setupUi(self):
        self.setObjectName("data_wiewer_widget")
        self.resize(710, 630)
        self.widget_layout = QtWidgets.QVBoxLayout(self)
        self.widget_layout.setContentsMargins(3, 3, 3, 3)
        self.widget_layout.setSpacing(3)
        self.widget_layout.setObjectName("widget_layout")
        self.groupBox = QtWidgets.QGroupBox(self)
        self.groupBox.setObjectName("groupBox")
        self.group_box_layout = QtWidgets.QVBoxLayout(self.groupBox)
        self.group_box_layout.setObjectName("group_box_layout")
        self.label_combo_box_layout = QtWidgets.QGridLayout()
        self.label_combo_box_layout.setObjectName("label_combo_box_layout")
        self.data_source_label = QtWidgets.QLabel("DataSource", self.groupBox)
        self.data_source_label.setObjectName("data_source_label")
        self.label_combo_box_layout.addWidget(self.data_source_label, 0, 0, 1, 1)
        self.data_source_combo_box = QtWidgets.QComboBox(self.groupBox)
        self.data_source_combo_box.setObjectName("data_source_combo_box")
        self.label_combo_box_layout.addWidget(self.data_source_combo_box, 0, 1, 1, 1)
        self.group_box_layout.addLayout(self.label_combo_box_layout)
        self.data_source_stacked_widget = QtWidgets.QStackedWidget(self.groupBox)
        self.data_source_stacked_widget.setObjectName("data_source_stacked_widget")
        self.group_box_layout.addWidget(self.data_source_stacked_widget)
        self.widget_layout.addWidget(self.groupBox)

        #self.page = DataSourcePage({})
        #self.data_source_stacked_widget.addWidget(self.page)

        QtCore.QMetaObject.connectSlotsByName(self)

    def add_data_model(self, data_model, data_model_dict):
        self.data_source_combo_box.addItem(data_model.name)
        page = DataSourcePage(data_model_dict)
        self.data_model_to_widget[data_model] = page
        self.data_source_stacked_widget.addWidget(page)
        self.data_source_stacked_widget.setCurrentIndex(0)
        self.setContentLayout(self.widget_layout)

    def remove_data_model(self, data_model):
        page_to_remove = self.data_model_to_widget[data_model]
        page = None
        for page_index in range(self.data_source_stacked_widget.count()):
            page = self.data_source_stacked_widget.widget(page_index)
            if page == page_to_remove:
                break
        self.data_source_combo_box.removeItem(page_index)
        self.data_source_stacked_widget.removeWidget(page_to_remove)
        self.data_model_to_widget.pop(data_model)
        if page:
            page.remove()
        #self.setContentLayout(self.widget_layout)

    def remove_all_data(self):
        for page_index in range(self.data_source_stacked_widget.count()).__reversed__():
            page = self.data_source_stacked_widget.widget(page_index)
            self.data_source_combo_box.removeItem(page_index)
            self.data_source_stacked_widget.removeWidget(page)
            page.remove()
        self.data_model_to_widget = {}


class DataSourcePage(QtWidgets.QWidget):
    def __init__(self, data_source_dict, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.create_ui()
        self.data_set_combo_box.currentTextChanged.connect(self.on_data_set_change)
        for data_set_name, data_set in data_source_dict.items():
            self.data_set_combo_box.addItem(data_set_name)
            data_set_page = DataSetPage(data_set)
            self.data_set_stacked_widget.addWidget(data_set_page)

    def on_data_set_change(self):
        data_set_index = self.data_set_combo_box.currentIndex()
        self.data_set_stacked_widget.setCurrentIndex(data_set_index)

    def create_ui(self):
        self.data_source_page_layout = QtWidgets.QVBoxLayout(self)
        self.data_source_page_layout.setContentsMargins(0, 0, 0, 0)
        self.data_source_page_layout.setSpacing(6)
        self.data_source_page_layout.setObjectName("verticalLayout_4")
        self.data_set_only_curr_scan_layout = QtWidgets.QGridLayout()
        self.data_set_only_curr_scan_layout.setSpacing(6)
        self.data_set_only_curr_scan_layout.setObjectName("data_set_only_curr_scan_layout")
        self.data_set_label = QtWidgets.QLabel("Data set", self)
        self.data_set_label.setObjectName("data_set_label")
        self.data_set_only_curr_scan_layout.addWidget(self.data_set_label, 0, 0, 1, 1)
        self.data_set_combo_box = QtWidgets.QComboBox(self)
        self.data_set_combo_box.setObjectName("data_set_combo_box")
        self.data_set_only_curr_scan_layout.addWidget(self.data_set_combo_box, 0, 1, 1, 1)
        self.only_curr_scan_label = QtWidgets.QLabel("Only current scan", self)
        self.only_curr_scan_label.setObjectName("only_curr_scan_label")
        self.data_set_only_curr_scan_layout.addWidget(self.only_curr_scan_label, 1, 0, 1, 1)
        self.current_scan_check_box = QtWidgets.QCheckBox(self)
        self.current_scan_check_box.setText("")
        self.current_scan_check_box.setObjectName("current_scan_check_box")
        self.data_set_only_curr_scan_layout.addWidget(self.current_scan_check_box, 1, 1, 1, 1)
        self.data_source_page_layout.addLayout(self.data_set_only_curr_scan_layout)
        self.data_set_stacked_widget = QtWidgets.QStackedWidget(self)
        self.data_set_stacked_widget.setObjectName("data_set_stacked_widget")
        self.data_source_page_layout.addWidget(self.data_set_stacked_widget)

    def remove(self):
        for widget_index in range(self.data_set_stacked_widget.count()).__reversed__():
            ds_page = self.data_set_stacked_widget.widget(widget_index)
            ds_page.remove()
        self.setParent(None)


class DataSetPage(QtWidgets.QWidget):
    def __init__(self, data_set_dict, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.create_ui()
        for tab_name, tab_data in data_set_dict.items():
            self.add_tab_with_table(tab_name, tab_data)

    def create_ui(self):
        self.verticalLayout = QtWidgets.QVBoxLayout(self)
        self.verticalLayout.setContentsMargins(0, 3, 0, 0)
        self.verticalLayout.setSpacing(0)
        self.verticalLayout.setObjectName("verticalLayout")
        self.tabWidget = QtWidgets.QTabWidget(self)
        self.tabWidget.setObjectName("tabWidget")
        self.verticalLayout.addWidget(self.tabWidget)

    def add_tab_with_table(self, name, data_frame):
        tab = QtWidgets.QWidget()
        tab_layout = QtWidgets.QVBoxLayout(tab)
        tab_layout.setContentsMargins(3, 3, 3, 3)
        table = QtWidgets.QTableView(tab)
        df_model = DataFrameTableModel(data_frame, tab)
        table.setModel(df_model)
        tab_layout.addWidget(table)

        self.tabWidget.addTab(tab, name)

    def add_table_row(self, table, pd_series):
        row_pos = table.rowCount()
        table.insertRow(row_pos)
        for col_pos, value in enumerate(pd_series):
            if isinstance(value, Enum):
                table.setItem(row_pos, col_pos, QtGui.QTableWidgetItem(value.name))
            elif isinstance(value, (np.ndarray, int)):
                table.setItem(row_pos, col_pos, QtGui.QTableWidgetItem(f"{value}"))
            elif isinstance(value, str):
                table.setItem(row_pos, col_pos, QtGui.QTableWidgetItem(value))
            else:
                table.setItem(row_pos, col_pos, QtGui.QTableWidgetItem(f"{value:9.4f}"))

    def remove(self):
        for widget_index in range(self.tabWidget.count()):
            widget = self.tabWidget.widget(widget_index)
            for child in widget.children():
                if isinstance(child, QtWidgets.QTableView):
                    child.setModel(None)
                    child.setParent(None)
                elif isinstance(child, DataFrameTableModel):
                    child.remove()
        self.setParent(None)
