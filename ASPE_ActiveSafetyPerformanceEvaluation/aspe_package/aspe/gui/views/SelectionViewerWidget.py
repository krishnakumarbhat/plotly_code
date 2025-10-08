from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QMenu

from aspe.gui.views.CollapsibleBoxWidget import CollapsibleBox


class SelectionViewerWidget(CollapsibleBox):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.controller = None
        self.setupUi()
        self.selection_table.setAlternatingRowColors(True)
        self.currently_selected = None

    def setupUi(self):
        self.widget_layout = QtWidgets.QVBoxLayout()
        self.widget_layout.setContentsMargins(3, 3, 3, 3)

        self.groupBox = QtWidgets.QGroupBox(self)
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.groupBox)
        self.verticalLayout_2.setContentsMargins(3, 3, 3, 3)

        self.gridLayout = QtWidgets.QGridLayout()
        self.data_set_name = QtWidgets.QLabel(self.groupBox)
        self.gridLayout.addWidget(self.data_set_name, 1, 1, 1, 1)
        self.data_set_label = QtWidgets.QLabel(self.groupBox)
        self.gridLayout.addWidget(self.data_set_label, 1, 0, 1, 1)
        self.data_source_name = QtWidgets.QLabel(self.groupBox)
        self.gridLayout.addWidget(self.data_source_name, 0, 1, 1, 1)
        self.data_source_label = QtWidgets.QLabel(self.groupBox)
        self.gridLayout.addWidget(self.data_source_label, 0, 0, 1, 1)
        self.verticalLayout_2.addLayout(self.gridLayout)
        self.selection_table = QtWidgets.QTableWidget(self.groupBox)
        self.selection_table.setEditTriggers(QtGui.QAbstractItemView.NoEditTriggers)
        self.selection_table.setMinimumHeight(500)
        self.selection_table.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)
        self.selection_table.setColumnCount(2)
        self.selection_table.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        self.selection_table.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        self.selection_table.setHorizontalHeaderItem(1, item)
        self.selection_table.horizontalHeader().setVisible(True)
        self.selection_table.horizontalHeader().setDefaultSectionSize(150)
        self.selection_table.horizontalHeader().setHighlightSections(True)
        self.selection_table.horizontalHeader().setMinimumSectionSize(30)
        self.selection_table.horizontalHeader().setStretchLastSection(True)
        self.selection_table.verticalHeader().setVisible(False)
        self.selection_table.verticalHeader().setDefaultSectionSize(30)
        self.selection_table.verticalHeader().setStretchLastSection(False)
        self.verticalLayout_2.addWidget(self.selection_table)
        self.widget_layout.addWidget(self.groupBox)
        self.setContentLayout(self.widget_layout)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.setWindowTitle(_translate("selection_viewer", "Form"))
        self.groupBox.setTitle(_translate("selection_viewer", "SelectionView"))
        self.data_set_name.setText(_translate("selection_viewer", "-"))
        self.data_set_label.setText(_translate("selection_viewer", "dataSet"))
        self.data_source_name.setText(_translate("selection_viewer", "-"))
        self.data_source_label.setText(_translate("selection_viewer", "dataSource"))
        item = self.selection_table.horizontalHeaderItem(0)
        item.setText(_translate("selection_viewer", "signature"))
        item = self.selection_table.horizontalHeaderItem(1)
        item.setText(_translate("selection_viewer", "value"))

    def update_selection(self, data_source, data_set, selected_row):
        self.data_source_name.setText(data_source.data_model.source_info.root_folder)
        self.data_set_name.setText(data_set.name)
        if isinstance(self.currently_selected, type(data_set)):
            self.update_current_table(selected_row)
        else:
            self.currently_selected = data_set
            self.create_new_table(selected_row)

    def create_new_table(self, selected_row):
        self.selection_table.setRowCount(0)
        for signal_signature, signal_value in selected_row.items():
            self.add_row(signal_signature, signal_value)

    def add_row(self, signal_signature, signal_value):
        row_position = self.selection_table.rowCount()
        self.selection_table.insertRow(row_position)
        self.selection_table.setItem(row_position, 0, QtGui.QTableWidgetItem(signal_signature))
        self.selection_table.setItem(row_position, 1, QtGui.QTableWidgetItem(str(signal_value)))
        self.selection_table.setRowHeight(row_position, 10)

    def update_current_table(self, selected_row):
        for signal_signature, signal_value in selected_row.items():
            items = self.selection_table.findItems(signal_signature, Qt.MatchExactly)
            item_row = items[0].row()
            item_to_edit = self.selection_table.item(item_row, 1)
            item_to_edit.setText(str(signal_value))

    def contextMenuEvent(self, event):
        if self.currently_selected is not None:  # don't trigger context menu when nothing is selected
            menu = QMenu(self.selection_table)
            self.controller.on_selection_viewer_context_menu_event(event, menu, self)

    def get_selected_signals_signatures(self):
        selected_rows = self.selection_table.selectionModel().selectedRows()
        selected_rows_idxs = [r.row() for r in selected_rows]
        return [self.selection_table.item(row_idx, 0).text() for row_idx in selected_rows_idxs]

    def on_deselect(self):
        self.currently_selected = None
        self.selection_table.setRowCount(0)
        self.data_set_name.setText("-")
        self.data_source_name.setText("-")