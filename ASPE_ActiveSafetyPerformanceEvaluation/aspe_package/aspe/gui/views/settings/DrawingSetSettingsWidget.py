from PyQt5 import QtCore, QtWidgets


class DrawingSetSettingsWidget(QtWidgets.QWidget):
    def __init__(self, data_set, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.parent_data_set = data_set
        self.setupUi()
        self.connect_signals()

    def connect_signals(self):
        self.enabled_check_box.stateChanged.connect(self.on_enabled_state_changed)

    def setupUi(self):
        self.setObjectName("Form")
        self.resize(320, 440)
        self.widget_layout = QtWidgets.QVBoxLayout(self)
        self.widget_layout.setContentsMargins(3, 3, 3, 3)
        self.is_enabled_panel = QtWidgets.QWidget(self)
        self.is_enabled_panel.setMaximumSize(QtCore.QSize(16777215, 35))
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.is_enabled_panel)
        self.enabled_label = QtWidgets.QLabel(self.is_enabled_panel)
        self.enabled_label.setText("enabled")
        self.horizontalLayout.addWidget(self.enabled_label)
        self.enabled_check_box = QtWidgets.QCheckBox(self.is_enabled_panel)
        self.enabled_check_box.setText("")
        self.enabled_check_box.setChecked(True)
        self.horizontalLayout.addWidget(self.enabled_check_box)
        self.widget_layout.addWidget(self.is_enabled_panel)

        self.drawers_setting_group_box = QtWidgets.QGroupBox(self)
        self.drawers_setting_group_box.setTitle("DrawersSettings")
        self.widget_layout.addWidget(self.drawers_setting_group_box)

        self.drawers_settings_layout = QtWidgets.QVBoxLayout(self.drawers_setting_group_box)
        self.drawers_settings_layout.setContentsMargins(3, 3, 3, 3)

        # SCROLL AREA
        self.scrollArea = QtWidgets.QScrollArea(self.drawers_setting_group_box)
        self.scrollArea.setWidgetResizable(True)
        self.drawers_settings_layout.addWidget(self.scrollArea)

        self.scroll_area_contents = QtWidgets.QWidget()
        self.scroll_area_contents.setGeometry(QtCore.QRect(0, 0, 238, 186))
        self.scrollArea.setWidget(self.scroll_area_contents)

        self.scroll_area_layout = QtWidgets.QVBoxLayout(self.scroll_area_contents)
        self.scroll_area_layout.setContentsMargins(3, 3, 3, 3)
        self.scroll_area_layout.setSpacing(3)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.setWindowTitle("Form")

    def add_drawer_setting(self, drawer_setting_widget):
        self.scroll_area_layout.addWidget(drawer_setting_widget)

    def on_enabled_state_changed(self):
        is_enabled = self.enabled_check_box.checkState()
        if is_enabled:
            self.drawers_setting_group_box.setEnabled(True)
            self.parent_data_set.enable()
        else:
            self.drawers_setting_group_box.setEnabled(False)
            self.parent_data_set.disable()

    def load_state(self, state):
        is_enabled = state["is_enabled"]
        self.enabled_check_box.setChecked(is_enabled)

    def remove(self):
        self.setParent(None)