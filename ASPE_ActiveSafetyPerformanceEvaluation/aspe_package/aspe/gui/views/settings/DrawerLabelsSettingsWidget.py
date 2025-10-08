from PyQt5 import QtCore, QtWidgets


class DrawerLabelsSettingsWidget(QtWidgets.QWidget):
    def __init__(self, name, drawer, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.drawer = drawer
        self.name = name
        self.setupUi()
        self.connect_signals()

    def setupUi(self):
        self.setObjectName("Form")
        self.resize(251, 169)
        self.widget_layout = QtWidgets.QVBoxLayout(self)
        self.widget_layout.setContentsMargins(3, 3, 3, 3)
        self.widget_layout.setSpacing(3)
        self.drawer_group_box = QtWidgets.QGroupBox(self)
        self.drawer_group_box.setTitle(self.name)
        self.gridLayout = QtWidgets.QGridLayout(self.drawer_group_box)
        self.gridLayout.setContentsMargins(3, 3, 3, 3)
        self.gridLayout.setSpacing(6)
        self.color_picker_button = QtWidgets.QPushButton(self.drawer_group_box)
        self.color_picker_button.setText("")
        self.gridLayout.addWidget(self.color_picker_button, 2, 1, 1, 1)
        self.signal_label = QtWidgets.QLabel(self.drawer_group_box)
        self.gridLayout.addWidget(self.signal_label, 3, 0, 1, 1)
        self.show_name_check_box = QtWidgets.QCheckBox(self.drawer_group_box)
        self.show_name_check_box.setText("")
        self.show_name_check_box.setChecked(True)
        self.gridLayout.addWidget(self.show_name_check_box, 1, 1, 1, 1)
        self.color_label = QtWidgets.QLabel(self.drawer_group_box)
        self.gridLayout.addWidget(self.color_label, 2, 0, 1, 1)
        self.is_enabled_check_box = QtWidgets.QCheckBox(self.drawer_group_box)
        self.is_enabled_check_box.setText("")
        self.is_enabled_check_box.setChecked(True)
        self.gridLayout.addWidget(self.is_enabled_check_box, 0, 1, 1, 1)
        self.include_signature_label = QtWidgets.QLabel(self.drawer_group_box)
        self.gridLayout.addWidget(self.include_signature_label, 1, 0, 1, 1)
        self.signal_combo_box = QtWidgets.QComboBox(self.drawer_group_box)
        self.gridLayout.addWidget(self.signal_combo_box, 3, 1, 1, 1)
        self.enabled_label = QtWidgets.QLabel(self.drawer_group_box)
        self.gridLayout.addWidget(self.enabled_label, 0, 0, 1, 1)
        self.font_size_label = QtWidgets.QLabel(self.drawer_group_box)
        self.gridLayout.addWidget(self.font_size_label, 4, 0, 1, 1)
        self.font_size_slider = QtWidgets.QSlider(self.drawer_group_box)
        self.font_size_slider.setMinimum(8)
        self.font_size_slider.setMaximum(20)
        self.font_size_slider.setOrientation(QtCore.Qt.Horizontal)
        self.gridLayout.addWidget(self.font_size_slider, 4, 1, 1, 1)
        self.gridLayout.setColumnStretch(0, 1)
        self.gridLayout.setColumnStretch(1, 1)
        self.widget_layout.addWidget(self.drawer_group_box)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.setWindowTitle(_translate("Form", "Form"))
        self.signal_label.setText(_translate("Form", "signal"))
        self.color_label.setText(_translate("Form", "color"))
        self.include_signature_label.setText(_translate("Form", "Show name"))
        self.enabled_label.setText(_translate("Form", "enabled"))
        self.font_size_label.setText(_translate("Form", "font size"))

    def fill_signals_combo_box(self, signals_names):
        for signal in signals_names:
            self.signal_combo_box.addItem(signal)

    def connect_signals(self):
        self.is_enabled_check_box.stateChanged.connect(self.on_enabled_state_changed)
        self.color_picker_button.clicked.connect(self.on_color_button_click)
        self.show_name_check_box.stateChanged.connect(self.on_show_signature_state_changed)
        self.signal_combo_box.currentTextChanged.connect(self.on_signal_change)
        self.font_size_slider.valueChanged.connect(self.on_font_size_change)

    def on_enabled_state_changed(self):
        is_enabled = self.is_enabled_check_box.isChecked()
        if is_enabled:
            self.drawer.enable()
        else:
            self.drawer.disable()

    def on_color_button_click(self):
        color = QtWidgets.QColorDialog.getColor().name()
        self.color_picker_button.setStyleSheet(f"background-color: {color};")
        self.drawer.set_color(color)

    def on_show_signature_state_changed(self):
        show_signature = self.show_name_check_box.isChecked()
        self.drawer.set_show_name(show_signature)

    def on_signal_change(self):
        signal = self.signal_combo_box.currentText()
        self.drawer.set_labeled_signal(signal)

    def on_font_size_change(self):
        font_size = self.font_size_slider.value()
        self.drawer.set_font_size(font_size)

    def remove(self):
        self.setParent(None)

    def load_state(self, state):
        is_enabled = state["is_enabled"]
        color = state["color"]
        self.is_enabled_check_box.setChecked(is_enabled)
        self.color_picker_button.setStyleSheet(f"background-color: {color};")
        self.signal_combo_box.setCurrentText(state["labeled_signal"])
        self.show_name_check_box.setChecked(state["show_signal_name"])
        self.font_size_slider.setValue(state["font_size"])
