# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\points_drawer_settings.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtWidgets


class Ui_points_drawer_settings(object):
    def setupUi(self, points_drawer_settings):
        points_drawer_settings.setObjectName("points_drawer_settings")
        points_drawer_settings.resize(224, 155)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Maximum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(points_drawer_settings.sizePolicy().hasHeightForWidth())
        points_drawer_settings.setSizePolicy(sizePolicy)
        self.gridLayout_3 = QtWidgets.QGridLayout(points_drawer_settings)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.drawer_setting_group_box = QtWidgets.QGroupBox(points_drawer_settings)
        self.drawer_setting_group_box.setObjectName("drawer_setting_group_box")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.drawer_setting_group_box)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.gridLayout = QtWidgets.QGridLayout()
        self.gridLayout.setObjectName("gridLayout")
        self.symbol_label = QtWidgets.QLabel(self.drawer_setting_group_box)
        self.symbol_label.setObjectName("symbol_label")
        self.gridLayout.addWidget(self.symbol_label, 2, 0, 1, 1)
        self.enabled_label = QtWidgets.QLabel(self.drawer_setting_group_box)
        self.enabled_label.setObjectName("enabled_label")
        self.gridLayout.addWidget(self.enabled_label, 0, 0, 1, 1)
        self.color_pickler_button = QtWidgets.QPushButton(self.drawer_setting_group_box)
        self.color_pickler_button.setText("")
        self.color_pickler_button.setObjectName("color_pickler_button")
        self.gridLayout.addWidget(self.color_pickler_button, 1, 1, 1, 1)
        self.is_enabled_check_box = QtWidgets.QCheckBox(self.drawer_setting_group_box)
        self.is_enabled_check_box.setText("")
        self.is_enabled_check_box.setObjectName("is_enabled_check_box")
        self.gridLayout.addWidget(self.is_enabled_check_box, 0, 1, 1, 1)
        self.color_label = QtWidgets.QLabel(self.drawer_setting_group_box)
        self.color_label.setObjectName("color_label")
        self.gridLayout.addWidget(self.color_label, 1, 0, 1, 1)
        self.symbol_combo_box = QtWidgets.QComboBox(self.drawer_setting_group_box)
        self.symbol_combo_box.setObjectName("symbol_combo_box")
        self.symbol_combo_box.addItem("")
        self.symbol_combo_box.addItem("")
        self.symbol_combo_box.addItem("")
        self.symbol_combo_box.addItem("")
        self.symbol_combo_box.addItem("")
        self.symbol_combo_box.addItem("")
        self.symbol_combo_box.addItem("")
        self.symbol_combo_box.addItem("")
        self.symbol_combo_box.addItem("")
        self.symbol_combo_box.addItem("")
        self.gridLayout.addWidget(self.symbol_combo_box, 2, 1, 1, 1)
        self.symbol_size_label = QtWidgets.QLabel(self.drawer_setting_group_box)
        self.symbol_size_label.setObjectName("symbol_size_label")
        self.gridLayout.addWidget(self.symbol_size_label, 3, 0, 1, 1)
        self.symbol_size_slider = QtWidgets.QSlider(self.drawer_setting_group_box)
        self.symbol_size_slider.setMinimum(1)
        self.symbol_size_slider.setMaximum(20)
        self.symbol_size_slider.setOrientation(QtCore.Qt.Horizontal)
        self.symbol_size_slider.setObjectName("symbol_size_slider")
        self.gridLayout.addWidget(self.symbol_size_slider, 3, 1, 1, 1)
        self.gridLayout.setColumnStretch(0, 1)
        self.gridLayout.setColumnStretch(1, 1)
        self.gridLayout_2.addLayout(self.gridLayout, 0, 0, 1, 1)
        self.gridLayout_3.addWidget(self.drawer_setting_group_box, 0, 0, 1, 1)

        self.retranslateUi(points_drawer_settings)
        QtCore.QMetaObject.connectSlotsByName(points_drawer_settings)

    def retranslateUi(self, points_drawer_settings):
        _translate = QtCore.QCoreApplication.translate
        points_drawer_settings.setWindowTitle(_translate("points_drawer_settings", "Form"))
        self.drawer_setting_group_box.setTitle(_translate("points_drawer_settings", "DrawerName"))
        self.symbol_label.setText(_translate("points_drawer_settings", "symbol"))
        self.enabled_label.setText(_translate("points_drawer_settings", "enabled"))
        self.color_label.setText(_translate("points_drawer_settings", "color"))
        self.symbol_combo_box.setItemText(0, _translate("points_drawer_settings", "o"))
        self.symbol_combo_box.setItemText(1, _translate("points_drawer_settings", "s"))
        self.symbol_combo_box.setItemText(2, _translate("points_drawer_settings", "+"))
        self.symbol_combo_box.setItemText(3, _translate("points_drawer_settings", "d"))
        self.symbol_combo_box.setItemText(4, _translate("points_drawer_settings", "t1"))
        self.symbol_combo_box.setItemText(5, _translate("points_drawer_settings", "t2"))
        self.symbol_combo_box.setItemText(6, _translate("points_drawer_settings", "t3"))
        self.symbol_combo_box.setItemText(7, _translate("points_drawer_settings", "p"))
        self.symbol_combo_box.setItemText(8, _translate("points_drawer_settings", "h"))
        self.symbol_combo_box.setItemText(9, _translate("points_drawer_settings", "star"))
        self.symbol_size_label.setText(_translate("points_drawer_settings", "symbol size"))
