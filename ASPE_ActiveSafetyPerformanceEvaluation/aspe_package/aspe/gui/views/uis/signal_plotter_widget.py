# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\signal_plotter_widget.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtWidgets


class Ui_SignalPloterWidget(object):
    def setupUi(self, SignalPloterWidget):
        SignalPloterWidget.setObjectName("SignalPloterWidget")
        SignalPloterWidget.resize(764, 237)
        self.horizontalLayout = QtWidgets.QHBoxLayout(SignalPloterWidget)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.axes_widget = QtWidgets.QWidget(SignalPloterWidget)
        self.axes_widget.setObjectName("axes_widget")
        self.axes_vlayout = QtWidgets.QVBoxLayout(self.axes_widget)
        self.axes_vlayout.setObjectName("axes_vlayout")
        self.horizontalLayout.addWidget(self.axes_widget)
        self.right_panel_widget = QtWidgets.QWidget(SignalPloterWidget)
        self.right_panel_widget.setObjectName("right_panel_widget")
        self.righ_panel_vlayout = QtWidgets.QVBoxLayout(self.right_panel_widget)
        self.righ_panel_vlayout.setObjectName("righ_panel_vlayout")
        self.signal_info_table = QtWidgets.QTableWidget(self.right_panel_widget)
        self.signal_info_table.setObjectName("signal_info_table")
        self.signal_info_table.setColumnCount(3)
        self.signal_info_table.setRowCount(0)
        item = QtWidgets.QTableWidgetItem()
        self.signal_info_table.setHorizontalHeaderItem(0, item)
        item = QtWidgets.QTableWidgetItem()
        self.signal_info_table.setHorizontalHeaderItem(1, item)
        item = QtWidgets.QTableWidgetItem()
        self.signal_info_table.setHorizontalHeaderItem(2, item)
        self.signal_info_table.horizontalHeader().setDefaultSectionSize(85)
        self.signal_info_table.horizontalHeader().setMinimumSectionSize(10)
        self.signal_info_table.horizontalHeader().setStretchLastSection(True)
        self.righ_panel_vlayout.addWidget(self.signal_info_table)
        self.horizontalLayout.addWidget(self.right_panel_widget)
        self.horizontalLayout.setStretch(0, 8)
        self.horizontalLayout.setStretch(1, 5)

        self.retranslateUi(SignalPloterWidget)
        QtCore.QMetaObject.connectSlotsByName(SignalPloterWidget)

    def retranslateUi(self, SignalPloterWidget):
        _translate = QtCore.QCoreApplication.translate
        SignalPloterWidget.setWindowTitle(_translate("SignalPloterWidget", "Form"))
        item = self.signal_info_table.horizontalHeaderItem(0)
        item.setText(_translate("SignalPloterWidget", "source"))
        item = self.signal_info_table.horizontalHeaderItem(1)
        item.setText(_translate("SignalPloterWidget", "signature"))
        item = self.signal_info_table.horizontalHeaderItem(2)
        item.setText(_translate("SignalPloterWidget", "value"))
