# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\right_panel_test.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtWidgets


class Ui_form_widget(object):
    def setupUi(self, form_widget):
        form_widget.setObjectName("form_widget")
        form_widget.resize(535, 658)
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(form_widget)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.child_widget = QtWidgets.QWidget(form_widget)
        self.child_widget.setObjectName("child_widget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.child_widget)
        self.verticalLayout.setObjectName("verticalLayout")
        self.group_box_1 = QtWidgets.QGroupBox(self.child_widget)
        self.group_box_1.setObjectName("group_box_1")
        self.pushButton = QtWidgets.QPushButton(self.group_box_1)
        self.pushButton.setGeometry(QtCore.QRect(100, 70, 75, 23))
        self.pushButton.setObjectName("pushButton")
        self.verticalLayout.addWidget(self.group_box_1)
        self.group_box_2 = QtWidgets.QGroupBox(self.child_widget)
        self.group_box_2.setObjectName("group_box_2")
        self.pushButton_2 = QtWidgets.QPushButton(self.group_box_2)
        self.pushButton_2.setGeometry(QtCore.QRect(40, 30, 75, 23))
        self.pushButton_2.setObjectName("pushButton_2")
        self.verticalLayout.addWidget(self.group_box_2)
        self.verticalLayout_2.addWidget(self.child_widget)

        self.retranslateUi(form_widget)
        QtCore.QMetaObject.connectSlotsByName(form_widget)

    def retranslateUi(self, form_widget):
        _translate = QtCore.QCoreApplication.translate
        form_widget.setWindowTitle(_translate("form_widget", "Form"))
        self.group_box_1.setTitle(_translate("form_widget", "GroupBox"))
        self.pushButton.setText(_translate("form_widget", "PushButton"))
        self.group_box_2.setTitle(_translate("form_widget", "GroupBox"))
        self.pushButton_2.setText(_translate("form_widget", "PushButton"))
