# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'C:\wkspaces\aspe_gui\views\uis\aspe_gui_main_window.ui'
#
# Created by: PyQt5 UI code generator 5.14.2
#
# WARNING! All changes made in this file will be lost!
from PyQt5 import QtCore, QtWidgets

class Ui_AspeGuiMainWindow(QtWidgets.QMainWindow):
    def setupUi(self):
        self.setObjectName("AspeGuiMainWindow")
        self.resize(1200, 800)
        self.setWindowState(QtCore.Qt.WindowMaximized)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(5)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.sizePolicy().hasHeightForWidth())
        self.setSizePolicy(sizePolicy)
        self.centralwidget = QtWidgets.QWidget(self)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.gridLayout = QtWidgets.QGridLayout()
        self.gridLayout.setObjectName("gridLayout")
        self.left_half_widget = QtWidgets.QWidget(self.centralwidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(3)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.left_half_widget.sizePolicy().hasHeightForWidth())
        self.left_half_widget.setSizePolicy(sizePolicy)
        self.left_half_widget.setObjectName("left_half_widget")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.left_half_widget)
        self.verticalLayout.setContentsMargins(0,0,0,0)

        self.verticalLayout.setObjectName("verticalLayout")
        self.control_panel_widget = QtWidgets.QWidget(self.left_half_widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(1)
        sizePolicy.setHeightForWidth(self.control_panel_widget.sizePolicy().hasHeightForWidth())
        self.control_panel_widget.setSizePolicy(sizePolicy)
        self.control_panel_widget.setObjectName("control_panel_widget")
        self.verticalLayout_3 = QtWidgets.QVBoxLayout(self.control_panel_widget)
        self.verticalLayout_3.setContentsMargins(0,0,0,0)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.control_panel_layout = QtWidgets.QVBoxLayout()
        self.control_panel_layout.setObjectName("control_panel_layout")
        self.control_panel_layout.setContentsMargins(0,0,0,0)
        self.verticalLayout_3.addLayout(self.control_panel_layout)
        self.verticalLayout.addWidget(self.control_panel_widget)
        self.bird_view_widget = QtWidgets.QWidget(self.left_half_widget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(3)
        sizePolicy.setHeightForWidth(self.bird_view_widget.sizePolicy().hasHeightForWidth())
        self.bird_view_widget.setSizePolicy(sizePolicy)
        self.bird_view_widget.setObjectName("bird_view_widget")
        self.gridLayout_5 = QtWidgets.QGridLayout(self.bird_view_widget)
        self.gridLayout_5.setObjectName("gridLayout_5")
        self.bird_view_layout = QtWidgets.QGridLayout()
        self.bird_view_layout.setObjectName("bird_view_layout")
        self.gridLayout_5.addLayout(self.bird_view_layout, 0, 0, 1, 1)
        self.verticalLayout.addWidget(self.bird_view_widget)
        self.gridLayout.addWidget(self.left_half_widget, 0, 0, 1, 1)
        self.right_half_widget = QtWidgets.QWidget(self.centralwidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(1)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.right_half_widget.sizePolicy().hasHeightForWidth())
        self.right_half_widget.setSizePolicy(sizePolicy)
        self.right_half_widget.setObjectName("right_half_widget")
        self.right_half_widget_layout = QtWidgets.QVBoxLayout(self.right_half_widget)
        self.tools_group_box = QtWidgets.QGroupBox(self.right_half_widget)
        self.tools_group_box.setTitle('ToolboxPanel')
        self.right_half_widget_layout.addWidget(self.tools_group_box)

        self.tools_panel_layout = QtWidgets.QVBoxLayout(self.tools_group_box)
        self.gridLayout.addWidget(self.right_half_widget, 0, 1, 1, 1)
        self.gridLayout_2.addLayout(self.gridLayout, 0, 0, 1, 1)
        self.setCentralWidget(self.centralwidget)

        # SCROLL AREA
        self.scrollArea = QtWidgets.QScrollArea(self.tools_group_box)
        self.scrollArea.setWidgetResizable(True)
        self.tools_panel_layout .addWidget(self.scrollArea)

        self.scroll_area_contents = QtWidgets.QWidget()
        self.scroll_area_contents.setGeometry(QtCore.QRect(0, 0, 238, 186))
        self.scrollArea.setWidget(self.scroll_area_contents)

        self.scroll_area_layout = QtWidgets.QVBoxLayout(self.scroll_area_contents)
        self.scroll_area_layout.setObjectName("scroll_area_layout")
        self.scroll_area_layout.setContentsMargins(3, 3, 3, 3)
        self.scroll_area_layout.setSpacing(3)

        self.menubar = QtWidgets.QMenuBar(self)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 962, 22))
        self.menubar.setObjectName("menubar")

        # TOOLBAR
        settingsMenu = QtWidgets.QMenu('&Settings', self)
        self.colorPresetsMenu = QtWidgets.QMenu('&Color presets', self)
        settingsMenu.addMenu(self.colorPresetsMenu)
        self.menubar.addMenu(settingsMenu)
        self.setMenuBar(self.menubar)

        self.statusbar = QtWidgets.QStatusBar(self)
        self.statusbar.setObjectName("statusbar")
        self.setStatusBar(self.statusbar)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.setWindowTitle(_translate("AspeGuiMainWindow", "ASPE GUI"))