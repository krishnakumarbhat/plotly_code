from PyQt5 import QtCore, QtWidgets


class PointsDrawerSettingsWidget(QtWidgets.QWidget):
    def __init__(self, name: str, points_drawer, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.name = name
        self.setupUi()
        self.points_drawer = points_drawer
        self.connect_signals()

    def setupUi(self):
        self.setObjectName("points_drawer_settings")
        self.resize(224, 155)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Maximum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.sizePolicy().hasHeightForWidth())
        self.setSizePolicy(sizePolicy)
        self.widget_layout = QtWidgets.QGridLayout(self)
        self.widget_layout.setObjectName("gridLayout_3")
        self.widget_layout.setContentsMargins(3, 3, 3, 3)

        self.drawer_setting_group_box = QtWidgets.QGroupBox(self)
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
        self.is_enabled_check_box.setChecked(True)
        self.gridLayout.addWidget(self.is_enabled_check_box, 0, 1, 1, 1)
        self.color_label = QtWidgets.QLabel(self.drawer_setting_group_box)
        self.color_label.setObjectName("color_label")
        self.gridLayout.addWidget(self.color_label, 1, 0, 1, 1)
        self.symbol_combo_box = QtWidgets.QComboBox(self.drawer_setting_group_box)
        self.symbol_combo_box.setObjectName("symbol_combo_box")
        self.symbol_combo_box.addItem("x")
        self.symbol_combo_box.addItem("o")
        self.symbol_combo_box.addItem("s")
        self.symbol_combo_box.addItem("+")
        self.symbol_combo_box.addItem("t1")
        self.symbol_combo_box.addItem("t2")
        self.symbol_combo_box.addItem("t3")
        self.symbol_combo_box.addItem("p")
        self.symbol_combo_box.addItem("h")
        self.symbol_combo_box.addItem("star")
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
        self.widget_layout.addWidget(self.drawer_setting_group_box, 0, 0, 1, 1)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.setWindowTitle(_translate("points_drawer_settings", "Form"))
        self.drawer_setting_group_box.setTitle(_translate("points_drawer_settings", self.name))
        self.symbol_label.setText(_translate("points_drawer_settings", "symbol"))
        self.enabled_label.setText(_translate("points_drawer_settings", "enabled"))
        self.color_label.setText(_translate("points_drawer_settings", "color"))
        self.symbol_size_label.setText(_translate("points_drawer_settings", "symbol size"))

    def connect_signals(self):
        self.is_enabled_check_box.stateChanged.connect(self.on_enabled_state_changed)
        self.color_pickler_button.clicked.connect(self.on_color_button_click)
        self.symbol_combo_box.currentTextChanged.connect(self.on_symbol_changed)
        self.symbol_size_slider.valueChanged.connect(self.on_symbol_size_changed)

    def on_enabled_state_changed(self):
        is_enabled = self.is_enabled_check_box.isChecked()
        if is_enabled:
            self.points_drawer.enable()
        else:
            self.points_drawer.disable()

    def on_color_button_click(self):
        color = QtWidgets.QColorDialog.getColor().name()
        self.color_pickler_button.setStyleSheet(f"background-color: {color};")
        self.points_drawer.change_line_color(color)

    def on_symbol_changed(self):
        symbol = self.symbol_combo_box.currentText()
        self.points_drawer.update_symbol(symbol)

    def on_symbol_size_changed(self):
        symbol_size = self.symbol_size_slider.value()
        self.points_drawer.update_symbol_size(symbol_size)

    def remove(self):
        self.setParent(None)

    def load_state(self, state):
        is_enabled = state["is_enabled"]
        color = state["color"]
        symbol = state["symbol"]
        symbol_size = state["symbol_size"]
        self.is_enabled_check_box.setChecked(is_enabled)
        self.color_pickler_button.setStyleSheet(f"background-color: {color};")
        self.symbol_combo_box.setCurrentText(symbol)
        self.symbol_size_slider.setValue(symbol_size)
