# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\data_set_settings.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtWidgets


class Ui_Form(object):
    def setupUi(self, Form):
        Form.setObjectName("Form")
        Form.resize(295, 256)
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(Form)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.widget = QtWidgets.QWidget(Form)
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
        self.verticalLayout_2.addWidget(self.widget)
        self.groupBox = QtWidgets.QGroupBox(Form)
        self.groupBox.setObjectName("groupBox")
        self.drawer_settings_group_box_layout = QtWidgets.QVBoxLayout(self.groupBox)
        self.drawer_settings_group_box_layout.setObjectName("drawer_settings_group_box_layout")
        self.scrollArea = QtWidgets.QScrollArea(self.groupBox)
        self.scrollArea.setWidgetResizable(True)
        self.scrollArea.setObjectName("scrollArea")
        self.scrollAreaWidgetContents_2 = QtWidgets.QWidget()
        self.scrollAreaWidgetContents_2.setGeometry(QtCore.QRect(0, 0, 238, 186))
        self.scrollAreaWidgetContents_2.setObjectName("scrollAreaWidgetContents_2")
        self.scroll_area_layout = QtWidgets.QVBoxLayout(self.scrollAreaWidgetContents_2)
        self.scroll_area_layout.setObjectName("scroll_area_layout")
        self.pushButton = QtWidgets.QPushButton(self.scrollAreaWidgetContents_2)
        self.pushButton.setObjectName("pushButton")
        self.scroll_area_layout.addWidget(self.pushButton)
        self.pushButton_3 = QtWidgets.QPushButton(self.scrollAreaWidgetContents_2)
        self.pushButton_3.setObjectName("pushButton_3")
        self.scroll_area_layout.addWidget(self.pushButton_3)
        self.pushButton_6 = QtWidgets.QPushButton(self.scrollAreaWidgetContents_2)
        self.pushButton_6.setObjectName("pushButton_6")
        self.scroll_area_layout.addWidget(self.pushButton_6)
        self.pushButton_5 = QtWidgets.QPushButton(self.scrollAreaWidgetContents_2)
        self.pushButton_5.setObjectName("pushButton_5")
        self.scroll_area_layout.addWidget(self.pushButton_5)
        self.pushButton_4 = QtWidgets.QPushButton(self.scrollAreaWidgetContents_2)
        self.pushButton_4.setObjectName("pushButton_4")
        self.scroll_area_layout.addWidget(self.pushButton_4)
        self.pushButton_2 = QtWidgets.QPushButton(self.scrollAreaWidgetContents_2)
        self.pushButton_2.setObjectName("pushButton_2")
        self.scroll_area_layout.addWidget(self.pushButton_2)
        self.scrollArea.setWidget(self.scrollAreaWidgetContents_2)
        self.drawer_settings_group_box_layout.addWidget(self.scrollArea)
        self.verticalLayout_2.addWidget(self.groupBox)

        self.retranslateUi(Form)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        _translate = QtCore.QCoreApplication.translate
        Form.setWindowTitle(_translate("Form", "Form"))
        self.label.setText(_translate("Form", "enabled"))
        self.groupBox.setTitle(_translate("Form", "DrawersSettings"))
        self.pushButton.setText(_translate("Form", "PushButton"))
        self.pushButton_3.setText(_translate("Form", "PushButton"))
        self.pushButton_6.setText(_translate("Form", "PushButton"))
        self.pushButton_5.setText(_translate("Form", "PushButton"))
        self.pushButton_4.setText(_translate("Form", "PushButton"))
        self.pushButton_2.setText(_translate("Form", "PushButton"))
