from PyQt5 import QtCore, QtWidgets

from aspe.gui.views.settings.DrawingSetSettingsWidget import DrawingSetSettingsWidget


class DrawerInternalObjectsSettings(DrawingSetSettingsWidget):
    def connect_signals(self):
        self.enabled_check_box.stateChanged.connect(self.on_enabled_state_changed)
        self.reduced_only_check_box.stateChanged.connect(self.on_reduced_only_state_changed)

    def setupUi(self):
        self.setObjectName("Form")
        self.resize(320, 440)
        self.widget_layout = QtWidgets.QVBoxLayout(self)
        self.widget_layout.setContentsMargins(3, 3, 3, 3)
        self.widget_layout.setObjectName("verticalLayout_2")

        self.is_enabled_panel = QtWidgets.QWidget(self)
        self.is_enabled_panel.setMaximumSize(QtCore.QSize(16777215, 35))
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.is_enabled_panel)
        self.enabled_label = QtWidgets.QLabel(self.is_enabled_panel)
        self.enabled_label.setText("enabled")

        self.horizontalLayout.addWidget(self.enabled_label)
        self.enabled_check_box = QtWidgets.QCheckBox(self.is_enabled_panel)
        self.enabled_check_box.setText("")
        self.enabled_check_box.setObjectName("checkBox")
        self.enabled_check_box.setChecked(True)
        self.horizontalLayout.addWidget(self.enabled_check_box)
        self.widget_layout.addWidget(self.is_enabled_panel)

        self.reduced_only_panel = QtWidgets.QWidget(self)
        self.reduced_only_panel.setMaximumSize(QtCore.QSize(16777215, 35))
        self.reduced_only_panel.setObjectName("widget")
        self.reduced_only_layout = QtWidgets.QHBoxLayout(self.reduced_only_panel)
        self.reduced_only_label = QtWidgets.QLabel(self.reduced_only_panel)
        self.reduced_only_label.setText("Reduced only")
        self.reduced_only_layout.addWidget(self.reduced_only_label)
        self.reduced_only_check_box = QtWidgets.QCheckBox(self.reduced_only_panel)
        self.reduced_only_check_box.setText("")
        self.reduced_only_check_box.setChecked(False)
        self.reduced_only_layout.addWidget(self.reduced_only_check_box)
        self.widget_layout.addWidget(self.reduced_only_panel)

        self.drawers_setting_group_box = QtWidgets.QGroupBox(self)
        self.drawers_setting_group_box.setTitle("DrawersSettings")
        self.widget_layout.addWidget(self.drawers_setting_group_box)

        self.drawers_settings_layout = QtWidgets.QVBoxLayout(self.drawers_setting_group_box)
        self.drawers_settings_layout.setObjectName("drawers_settings_layout")
        self.drawers_settings_layout.setContentsMargins(3, 3, 3, 3)

        # SCROLL AREA
        self.scrollArea = QtWidgets.QScrollArea(self.drawers_setting_group_box)
        self.scrollArea.setWidgetResizable(True)
        self.scrollArea.setObjectName("scrollArea")
        self.drawers_settings_layout.addWidget(self.scrollArea)

        self.scroll_area_contents = QtWidgets.QWidget()
        self.scroll_area_contents.setGeometry(QtCore.QRect(0, 0, 238, 186))
        self.scrollArea.setWidget(self.scroll_area_contents)

        self.scroll_area_layout = QtWidgets.QVBoxLayout(self.scroll_area_contents)
        self.scroll_area_layout.setObjectName("scroll_area_layout")
        self.scroll_area_layout.setContentsMargins(3, 3, 3, 3)
        self.scroll_area_layout.setSpacing(3)

        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def on_reduced_only_state_changed(self):
        reduced_only = self.reduced_only_check_box.checkState()
        if reduced_only:
            self.parent_data_set.switch_to_reduced()
        else:
            self.parent_data_set.switch_to_all_objects()

    def load_state(self, state):
        is_enabled = state["is_enabled"]
        only_reduced_objects = state["only_reduced_objects"]

        self.enabled_check_box.setChecked(is_enabled)
        self.reduced_only_check_box.setChecked(only_reduced_objects)