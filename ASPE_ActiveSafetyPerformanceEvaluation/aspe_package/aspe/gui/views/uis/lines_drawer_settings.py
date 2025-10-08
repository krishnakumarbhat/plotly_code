# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\lines_drawer_settings.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtWidgets


class Ui_lines_drawer_settings(object):
    def setupUi(self, lines_drawer_settings):
        lines_drawer_settings.setObjectName("lines_drawer_settings")
        lines_drawer_settings.resize(227, 150)
        self.gridLayout_3 = QtWidgets.QGridLayout(lines_drawer_settings)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.drawer_setting_group_box = QtWidgets.QGroupBox(lines_drawer_settings)
        self.drawer_setting_group_box.setObjectName("drawer_setting_group_box")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.drawer_setting_group_box)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.gridLayout = QtWidgets.QGridLayout()
        self.gridLayout.setObjectName("gridLayout")
        self.line_style_label = QtWidgets.QLabel(self.drawer_setting_group_box)
        self.line_style_label.setObjectName("line_style_label")
        self.gridLayout.addWidget(self.line_style_label, 2, 0, 1, 1)
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
        self.line_style_combo_box = QtWidgets.QComboBox(self.drawer_setting_group_box)
        self.line_style_combo_box.setObjectName("line_style_combo_box")
        self.line_style_combo_box.addItem("")
        self.line_style_combo_box.addItem("")
        self.line_style_combo_box.addItem("")
        self.line_style_combo_box.addItem("")
        self.line_style_combo_box.addItem("")
        self.gridLayout.addWidget(self.line_style_combo_box, 2, 1, 1, 1)
        self.line_width_label = QtWidgets.QLabel(self.drawer_setting_group_box)
        self.line_width_label.setObjectName("line_width_label")
        self.gridLayout.addWidget(self.line_width_label, 3, 0, 1, 1)
        self.line_width_slider = QtWidgets.QSlider(self.drawer_setting_group_box)
        self.line_width_slider.setMinimum(1)
        self.line_width_slider.setMaximum(10)
        self.line_width_slider.setOrientation(QtCore.Qt.Horizontal)
        self.line_width_slider.setObjectName("line_width_slider")
        self.gridLayout.addWidget(self.line_width_slider, 3, 1, 1, 1)
        self.gridLayout.setColumnStretch(0, 1)
        self.gridLayout.setColumnStretch(1, 1)
        self.gridLayout_2.addLayout(self.gridLayout, 0, 0, 1, 1)
        self.gridLayout_3.addWidget(self.drawer_setting_group_box, 0, 0, 1, 1)

        self.retranslateUi(lines_drawer_settings)
        QtCore.QMetaObject.connectSlotsByName(lines_drawer_settings)

    def retranslateUi(self, lines_drawer_settings):
        _translate = QtCore.QCoreApplication.translate
        lines_drawer_settings.setWindowTitle(_translate("lines_drawer_settings", "Form"))
        self.drawer_setting_group_box.setTitle(_translate("lines_drawer_settings", "DrawerName"))
        self.line_style_label.setText(_translate("lines_drawer_settings", "line_style"))
        self.enabled_label.setText(_translate("lines_drawer_settings", "enabled"))
        self.color_label.setText(_translate("lines_drawer_settings", "color"))
        self.line_style_combo_box.setItemText(0, _translate("lines_drawer_settings", "-"))
        self.line_style_combo_box.setItemText(1, _translate("lines_drawer_settings", "--"))
        self.line_style_combo_box.setItemText(2, _translate("lines_drawer_settings", "-*-"))
        self.line_style_combo_box.setItemText(3, _translate("lines_drawer_settings", "***"))
        self.line_style_combo_box.setItemText(4, _translate("lines_drawer_settings", "-**-"))
        self.line_width_label.setText(_translate("lines_drawer_settings", "line_width"))
