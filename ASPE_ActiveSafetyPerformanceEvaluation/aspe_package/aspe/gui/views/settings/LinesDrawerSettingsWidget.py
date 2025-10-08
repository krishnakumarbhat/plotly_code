from PyQt5 import QtCore, QtWidgets


class LinesDrawerSettingsWidget(QtWidgets.QWidget):
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
        self.widget_layout.setContentsMargins(3, 3, 3, 3)

        self.widget_layout.setObjectName("gridLayout_3")
        self.drawer_setting_group_box = QtWidgets.QGroupBox(self)
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
        self.is_enabled_check_box.setChecked(True)
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
        self.widget_layout.addWidget(self.drawer_setting_group_box, 0, 0, 1, 1)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.setWindowTitle(_translate("lines_drawer_settings", "Form"))
        self.drawer_setting_group_box.setTitle(_translate("lines_drawer_settings", self.name))
        self.line_style_label.setText(_translate("lines_drawer_settings", "line_style"))
        self.enabled_label.setText(_translate("lines_drawer_settings", "enabled"))
        self.color_label.setText(_translate("lines_drawer_settings", "color"))
        self.line_style_combo_box.setItemText(0, _translate("lines_drawer_settings", "-"))
        self.line_style_combo_box.setItemText(1, _translate("lines_drawer_settings", "--"))
        self.line_style_combo_box.setItemText(2, _translate("lines_drawer_settings", "-.-"))
        self.line_style_combo_box.setItemText(3, _translate("lines_drawer_settings", "..."))
        self.line_style_combo_box.setItemText(4, _translate("lines_drawer_settings", "-..-"))
        self.line_width_label.setText(_translate("lines_drawer_settings", "line_width"))

    def connect_signals(self):
        self.is_enabled_check_box.stateChanged.connect(self.on_enabled_state_changed)
        self.color_pickler_button.clicked.connect(self.on_color_button_click)
        self.line_style_combo_box.currentTextChanged.connect(self.on_line_style_change)
        self.line_width_slider.valueChanged.connect(self.on_line_width_change)

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

    def on_line_style_change(self):
        line_style = self.line_style_combo_box.currentText()
        self.lines_drawer.change_line_style(line_style)

    def on_line_width_change(self):
        width_value = self.line_width_slider.value()
        self.lines_drawer.change_line_width(width_value)

    def remove(self):
        self.setParent(None)

    def load_state(self, state):
        is_enabled = state["is_enabled"]
        color = state["color"]
        line_style = state["line_style"]
        line_width = state["line_width"]

        self.is_enabled_check_box.setChecked(is_enabled)
        self.color_pickler_button.setStyleSheet(f"background-color: {color};")
        self.line_style_combo_box.setCurrentText(line_style)
        self.line_width_slider.setValue(line_width)
