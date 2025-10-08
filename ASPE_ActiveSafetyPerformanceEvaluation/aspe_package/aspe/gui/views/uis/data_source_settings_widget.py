# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\data_source_settings_widget.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtWidgets


class Ui_data_source_settings_widget(object):
    def setupUi(self, data_source_settings_widget):
        data_source_settings_widget.setObjectName("data_source_settings_widget")
        data_source_settings_widget.resize(349, 365)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Maximum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(data_source_settings_widget.sizePolicy().hasHeightForWidth())
        data_source_settings_widget.setSizePolicy(sizePolicy)
        data_source_settings_widget.setMaximumSize(QtCore.QSize(450, 500))
        self.gridLayout = QtWidgets.QGridLayout(data_source_settings_widget)
        self.gridLayout.setObjectName("gridLayout")
        self.gridLayout_2 = QtWidgets.QGridLayout()
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.data_source_is_enabled_check_box = QtWidgets.QCheckBox(data_source_settings_widget)
        self.data_source_is_enabled_check_box.setText("")
        self.data_source_is_enabled_check_box.setObjectName("data_source_is_enabled_check_box")
        self.gridLayout_2.addWidget(self.data_source_is_enabled_check_box, 0, 1, 1, 1)
        self.data_source_enabled_label = QtWidgets.QLabel(data_source_settings_widget)
        self.data_source_enabled_label.setObjectName("data_source_enabled_label")
        self.gridLayout_2.addWidget(self.data_source_enabled_label, 0, 0, 1, 1)
        self.gridLayout.addLayout(self.gridLayout_2, 1, 0, 1, 1)
        self.data_set_settings_tab = QtWidgets.QTabWidget(data_source_settings_widget)
        self.data_set_settings_tab.setObjectName("data_set_settings_tab")
        self.data_set_1_tab = QtWidgets.QWidget()
        self.data_set_1_tab.setObjectName("data_set_1_tab")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.data_set_1_tab)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.gridLayout_3 = QtWidgets.QGridLayout()
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.data_set_enabled_label = QtWidgets.QLabel(self.data_set_1_tab)
        self.data_set_enabled_label.setObjectName("data_set_enabled_label")
        self.gridLayout_3.addWidget(self.data_set_enabled_label, 0, 0, 1, 1)
        self.data_set_enabled_check_box = QtWidgets.QCheckBox(self.data_set_1_tab)
        self.data_set_enabled_check_box.setText("")
        self.data_set_enabled_check_box.setObjectName("data_set_enabled_check_box")
        self.gridLayout_3.addWidget(self.data_set_enabled_check_box, 0, 1, 1, 1)
        self.verticalLayout_2.addLayout(self.gridLayout_3)
        self.data_set_settings_tab.addTab(self.data_set_1_tab, "")
        self.tab_2 = QtWidgets.QWidget()
        self.tab_2.setObjectName("tab_2")
        self.data_set_settings_tab.addTab(self.tab_2, "")
        self.gridLayout.addWidget(self.data_set_settings_tab, 6, 0, 1, 1)

        self.retranslateUi(data_source_settings_widget)
        self.data_set_settings_tab.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(data_source_settings_widget)

    def retranslateUi(self, data_source_settings_widget):
        _translate = QtCore.QCoreApplication.translate
        data_source_settings_widget.setWindowTitle(_translate("data_source_settings_widget", "Form"))
        self.data_source_enabled_label.setText(_translate("data_source_settings_widget", "enabled"))
        self.data_set_enabled_label.setText(_translate("data_source_settings_widget", "enabled"))
        self.data_set_settings_tab.setTabText(self.data_set_settings_tab.indexOf(self.data_set_1_tab), _translate("data_source_settings_widget", "Tab 1"))
        self.data_set_settings_tab.setTabText(self.data_set_settings_tab.indexOf(self.tab_2), _translate("data_source_settings_widget", "Tab 2"))
