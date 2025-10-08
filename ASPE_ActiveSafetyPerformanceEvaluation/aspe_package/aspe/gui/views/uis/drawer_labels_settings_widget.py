# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\drawer_labels_settings_widget.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtWidgets


class Ui_Form(object):
    def setupUi(self, Form):
        Form.setObjectName("Form")
        Form.resize(251, 169)
        self.verticalLayout = QtWidgets.QVBoxLayout(Form)
        self.verticalLayout.setContentsMargins(3, 3, 3, 3)
        self.verticalLayout.setSpacing(3)
        self.verticalLayout.setObjectName("verticalLayout")
        self.drawer_group_box = QtWidgets.QGroupBox(Form)
        self.drawer_group_box.setObjectName("drawer_group_box")
        self.gridLayout = QtWidgets.QGridLayout(self.drawer_group_box)
        self.gridLayout.setContentsMargins(3, 3, 3, 3)
        self.gridLayout.setSpacing(3)
        self.gridLayout.setObjectName("gridLayout")
        self.color_picker_button = QtWidgets.QPushButton(self.drawer_group_box)
        self.color_picker_button.setText("")
        self.color_picker_button.setObjectName("color_picker_button")
        self.gridLayout.addWidget(self.color_picker_button, 2, 1, 1, 1)
        self.signal_label = QtWidgets.QLabel(self.drawer_group_box)
        self.signal_label.setObjectName("signal_label")
        self.gridLayout.addWidget(self.signal_label, 3, 0, 1, 1)
        self.include_signature_check_box = QtWidgets.QCheckBox(self.drawer_group_box)
        self.include_signature_check_box.setText("")
        self.include_signature_check_box.setObjectName("include_signature_check_box")
        self.gridLayout.addWidget(self.include_signature_check_box, 1, 1, 1, 1)
        self.color_label = QtWidgets.QLabel(self.drawer_group_box)
        self.color_label.setObjectName("color_label")
        self.gridLayout.addWidget(self.color_label, 2, 0, 1, 1)
        self.enabled_check_box = QtWidgets.QCheckBox(self.drawer_group_box)
        self.enabled_check_box.setText("")
        self.enabled_check_box.setObjectName("enabled_check_box")
        self.gridLayout.addWidget(self.enabled_check_box, 0, 1, 1, 1)
        self.include_signature_label = QtWidgets.QLabel(self.drawer_group_box)
        self.include_signature_label.setObjectName("include_signature_label")
        self.gridLayout.addWidget(self.include_signature_label, 1, 0, 1, 1)
        self.signal_combo_box = QtWidgets.QComboBox(self.drawer_group_box)
        self.signal_combo_box.setObjectName("signal_combo_box")
        self.gridLayout.addWidget(self.signal_combo_box, 3, 1, 1, 1)
        self.enabled_label = QtWidgets.QLabel(self.drawer_group_box)
        self.enabled_label.setObjectName("enabled_label")
        self.gridLayout.addWidget(self.enabled_label, 0, 0, 1, 1)
        self.font_size_label = QtWidgets.QLabel(self.drawer_group_box)
        self.font_size_label.setObjectName("font_size_label")
        self.gridLayout.addWidget(self.font_size_label, 4, 0, 1, 1)
        self.font_size_slider = QtWidgets.QSlider(self.drawer_group_box)
        self.font_size_slider.setOrientation(QtCore.Qt.Horizontal)
        self.font_size_slider.setObjectName("font_size_slider")
        self.gridLayout.addWidget(self.font_size_slider, 4, 1, 1, 1)
        self.gridLayout.setColumnStretch(0, 2)
        self.gridLayout.setColumnStretch(1, 3)
        self.verticalLayout.addWidget(self.drawer_group_box)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        _translate = QtCore.QCoreApplication.translate
        Form.setWindowTitle(_translate("Form", "Form"))
        self.drawer_group_box.setTitle(_translate("Form", "DrawerName"))
        self.signal_label.setText(_translate("Form", "signal"))
        self.color_label.setText(_translate("Form", "color"))
        self.include_signature_label.setText(_translate("Form", "include signature"))
        self.enabled_label.setText(_translate("Form", "enabled"))
        self.font_size_label.setText(_translate("Form", "font size"))
