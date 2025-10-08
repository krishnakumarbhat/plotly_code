# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\data_source_settings.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtWidgets


class Ui_data_source_settings_widget(object):
    def setupUi(self, data_source_settings_widget):
        data_source_settings_widget.setObjectName("data_source_settings_widget")
        data_source_settings_widget.resize(450, 461)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Maximum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(data_source_settings_widget.sizePolicy().hasHeightForWidth())
        data_source_settings_widget.setSizePolicy(sizePolicy)
        data_source_settings_widget.setMaximumSize(QtCore.QSize(450, 500))
        self.verticalLayout = QtWidgets.QVBoxLayout(data_source_settings_widget)
        self.verticalLayout.setObjectName("verticalLayout")
        self.widget = QtWidgets.QWidget(data_source_settings_widget)
        self.widget.setMaximumSize(QtCore.QSize(16777215, 35))
        self.widget.setObjectName("widget")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.widget)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.label = QtWidgets.QLabel(self.widget)
        self.label.setObjectName("label")
        self.horizontalLayout.addWidget(self.label)
        self.checkBox = QtWidgets.QCheckBox(self.widget)
        self.checkBox.setText("")
        self.checkBox.setObjectName("checkBox")
        self.horizontalLayout.addWidget(self.checkBox)
        self.verticalLayout.addWidget(self.widget)
        self.tabWidget = QtWidgets.QTabWidget(data_source_settings_widget)
        self.tabWidget.setObjectName("tabWidget")
        self.tab = QtWidgets.QWidget()
        self.tab.setObjectName("tab")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.tab)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.tabWidget.addTab(self.tab, "")
        self.tab_2 = QtWidgets.QWidget()
        self.tab_2.setObjectName("tab_2")
        self.tabWidget.addTab(self.tab_2, "")
        self.verticalLayout.addWidget(self.tabWidget)

        self.retranslateUi(data_source_settings_widget)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(data_source_settings_widget)

    def retranslateUi(self, data_source_settings_widget):
        _translate = QtCore.QCoreApplication.translate
        data_source_settings_widget.setWindowTitle(_translate("data_source_settings_widget", "Form"))
        self.label.setText(_translate("data_source_settings_widget", "enabled"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), _translate("data_source_settings_widget", "Tab 1"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), _translate("data_source_settings_widget", "Tab 2"))
