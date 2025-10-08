# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\selection_view.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtWidgets


class Ui_selection_viewer:
    def setupUi(self, selection_viewer):
        selection_viewer.setObjectName("selection_viewer")
        selection_viewer.resize(297, 436)
        self.verticalLayout = QtWidgets.QVBoxLayout(selection_viewer)
        self.verticalLayout.setObjectName("verticalLayout")
        self.groupBox = QtWidgets.QGroupBox(selection_viewer)
        self.groupBox.setObjectName("groupBox")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.groupBox)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.gridLayout = QtWidgets.QGridLayout()
        self.gridLayout.setObjectName("gridLayout")
        self.data_set_name = QtWidgets.QLabel(self.groupBox)
        self.data_set_name.setObjectName("data_set_name")
        self.gridLayout.addWidget(self.data_set_name, 1, 1, 1, 1)
        self.data_set_label = QtWidgets.QLabel(self.groupBox)
        self.data_set_label.setObjectName("data_set_label")
        self.gridLayout.addWidget(self.data_set_label, 1, 0, 1, 1)
        self.data_source_name = QtWidgets.QLabel(self.groupBox)
        self.data_source_name.setObjectName("data_source_name")
        self.gridLayout.addWidget(self.data_source_name, 0, 1, 1, 1)
        self.data_source_label = QtWidgets.QLabel(self.groupBox)
        self.data_source_label.setObjectName("data_source_label")
        self.gridLayout.addWidget(self.data_source_label, 0, 0, 1, 1)
        self.verticalLayout_2.addLayout(self.gridLayout)
        self.tableWidget = QtWidgets.QTableWidget(self.groupBox)
        self.tableWidget.setSelectionMode(QtWidgets.QAbstractItemView.ExtendedSelection)
        self.tableWidget.setObjectName("tableWidget")
        self.tableWidget.setColumnCount(2)
        self.tableWidget.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        self.tableWidget.setHorizontalHeaderItem(1, item)
        self.tableWidget.horizontalHeader().setVisible(True)
        self.tableWidget.horizontalHeader().setDefaultSectionSize(150)
        self.tableWidget.horizontalHeader().setHighlightSections(True)
        self.tableWidget.horizontalHeader().setMinimumSectionSize(30)
        self.tableWidget.horizontalHeader().setStretchLastSection(True)
        self.tableWidget.verticalHeader().setVisible(False)
        self.tableWidget.verticalHeader().setDefaultSectionSize(30)
        self.tableWidget.verticalHeader().setStretchLastSection(False)
        self.verticalLayout_2.addWidget(self.tableWidget)
        self.verticalLayout.addWidget(self.groupBox)

        self.retranslateUi(selection_viewer)
        QtCore.QMetaObject.connectSlotsByName(selection_viewer)

    def retranslateUi(self, selection_viewer):
        _translate = QtCore.QCoreApplication.translate
        selection_viewer.setWindowTitle(_translate("selection_viewer", "Form"))
        self.groupBox.setTitle(_translate("selection_viewer", "SelectionView"))
        self.data_set_name.setText(_translate("selection_viewer", "TextLabel"))
        self.data_set_label.setText(_translate("selection_viewer", "dataSet"))
        self.data_source_name.setText(_translate("selection_viewer", "TextLabel"))
        self.data_source_label.setText(_translate("selection_viewer", "dataSource"))
        item = self.tableWidget.horizontalHeaderItem(0)
        item.setText(_translate("selection_viewer", "signature"))
        item = self.tableWidget.horizontalHeaderItem(1)
        item.setText(_translate("selection_viewer", "value"))
