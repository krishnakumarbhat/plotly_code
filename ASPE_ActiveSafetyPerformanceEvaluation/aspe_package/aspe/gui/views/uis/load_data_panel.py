# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\load_data_panel.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtWidgets
from aspe.gui.views.DataSourcesTable import DataSourcesTable


class Ui_LoadDataPanel(QtWidgets.QWidget):
    def setupUi(self):
        self.setObjectName("LoadDataPanel")
        self.setMaximumHeight(135)
        self.main_vlayout = QtWidgets.QVBoxLayout(self)

        self.main_vlayout.setObjectName("main_vlayout")
        self.main_vlayout.setContentsMargins(0,0,0,0)
        self.group_box = QtWidgets.QGroupBox(self)
        self.group_box.setObjectName("group_box")
        self.group_box_vlayout = QtWidgets.QVBoxLayout(self.group_box)
        self.group_box_vlayout.setObjectName("group_box_vlayout")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.log_path_combo_box = QtWidgets.QComboBox(self.group_box)
        self.log_path_combo_box.setEditable(True)
        self.log_path_combo_box.setObjectName("log_path_combo_box")
        self.horizontalLayout.addWidget(self.log_path_combo_box)
        self.open_file_dialog_button = QtWidgets.QPushButton(self.group_box)
        self.open_file_dialog_button.setObjectName("open_file_dialog_button")
        self.horizontalLayout.addWidget(self.open_file_dialog_button)
        self.data_sources_table = DataSourcesTable(self.group_box)
        self.horizontalLayout.addWidget(self.data_sources_table)
        self.horizontalLayout.setStretch(0, 8)
        self.horizontalLayout.setStretch(1, 1)
        self.horizontalLayout.setStretch(2, 10)
        self.group_box_vlayout.addLayout(self.horizontalLayout)
        self.main_vlayout.addWidget(self.group_box)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.setWindowTitle(_translate("LoadDataPanel", "Form"))
        self.group_box.setTitle(_translate("LoadDataPanel", "LoadDataPanel"))
        self.open_file_dialog_button.setText(_translate("LoadDataPanel", "Open"))
