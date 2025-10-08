from PyQt5 import QtCore, QtWidgets


class FilledPolygonDrawerSettingsWidget(QtWidgets.QWidget):
    def __init__(self, name: str, lines_drawer, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.name = name
        self.lines_drawer = lines_drawer
        self.setupUi()
        self.connect_signals()

    def setupUi(self):
        self.setObjectName("lines_drawer_settings")
        self.resize(227, 150)
        self.setMaximumHeight(150)
        self.widget_layout = QtWidgets.QGridLayout(self)
        self.widget_layout.setObjectName("gridLayout_3")
        self.widget_layout.setContentsMargins(3, 3, 3, 3)

        self.drawer_setting_group_box = QtWidgets.QGroupBox(self)
        self.drawer_setting_group_box.setObjectName("drawer_setting_group_box")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.drawer_setting_group_box)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.gridLayout = QtWidgets.QGridLayout()
        self.gridLayout.setObjectName("gridLayout")
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
        self.is_enabled_check_box.setChecked(True)
        self.gridLayout.addWidget(self.is_enabled_check_box, 0, 1, 1, 1)
        self.color_label = QtWidgets.QLabel(self.drawer_setting_group_box)
        self.color_label.setObjectName("color_label")
        self.gridLayout.addWidget(self.color_label, 1, 0, 1, 1)
        self.gridLayout.setColumnStretch(0, 1)
        self.gridLayout.setColumnStretch(1, 1)
        self.gridLayout_2.addLayout(self.gridLayout, 0, 0, 1, 1)
        self.widget_layout.addWidget(self.drawer_setting_group_box, 0, 0, 1, 1)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.setWindowTitle(_translate("lines_drawer_settings", "Form"))
        self.drawer_setting_group_box.setTitle(_translate("lines_drawer_settings", self.name))
        self.enabled_label.setText(_translate("lines_drawer_settings", "enabled"))
        self.color_label.setText(_translate("lines_drawer_settings", "color"))

    def connect_signals(self):
        self.is_enabled_check_box.stateChanged.connect(self.on_enabled_state_changed)
        self.color_pickler_button.clicked.connect(self.on_color_button_click)

    def on_enabled_state_changed(self):
        is_enabled = self.is_enabled_check_box.isChecked()
        if is_enabled:
            self.lines_drawer.enable()
        else:
            self.lines_drawer.disable()

    def on_color_button_click(self):
        color = QtWidgets.QColorDialog.getColor().name()
        self.color_pickler_button.setStyleSheet(f"background-color: {color};")
        self.lines_drawer.change_line_color(color)

    def remove(self):
        self.setParent(None)

    def load_state(self, state):
        is_enabled = state["is_enabled"]
        color = state["color"]
        self.is_enabled_check_box.setChecked(is_enabled)
        self.color_pickler_button.setStyleSheet(f"background-color: {color};")
