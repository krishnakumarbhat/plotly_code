from PyQt5 import QtCore, QtWidgets
from PyQt5.QtCore import Qt

from aspe.gui.views.CollapsibleBoxWidget import CollapsibleBox


class DrawersSettingsWidget(CollapsibleBox):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi()
        self.connect_signals()

    def connect_signals(self):
        self.sources_combo_box.currentIndexChanged.connect(self.on_data_source_index_changed)

    def on_data_source_index_changed(self):
        current_index = self.sources_combo_box.currentIndex()
        self.sources_stacked_widget.setCurrentIndex(current_index)

    def setupUi(self):
        self.setObjectName("Form")
        self.widget_layout = QtWidgets.QVBoxLayout()
        self.widget_layout.setContentsMargins(3, 3, 3, 3)
        self.groupBox = QtWidgets.QGroupBox(self)
        self.verticalLayout = QtWidgets.QVBoxLayout(self.groupBox)
        self.verticalLayout.setContentsMargins(3, 3, 3, 3)
        self.verticalLayout.setSpacing(0)

        self.widget = QtWidgets.QWidget(self.groupBox)
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.widget)
        self.horizontalLayout.setContentsMargins(3, 3, 3, 3)
        self.data_source_label = QtWidgets.QLabel("DataSource", self.widget)
        self.horizontalLayout.addWidget(self.data_source_label)
        self.sources_combo_box = QtWidgets.QComboBox(self.widget)
        self.horizontalLayout.addWidget(self.sources_combo_box)
        self.horizontalLayout.setStretch(0, 1)
        self.horizontalLayout.setStretch(1, 2)
        self.verticalLayout.addWidget(self.widget)
        self.sources_stacked_widget = QtWidgets.QStackedWidget(self.groupBox)

        self.verticalLayout.addWidget(self.sources_stacked_widget)
        self.widget_layout.addWidget(self.groupBox)
        self.setContentLayout(self.widget_layout)
        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate

    def add_data_source_settings(self, data_source_setting_widget, name="source"):
        self.sources_stacked_widget.addWidget(data_source_setting_widget)
        self.sources_combo_box.addItem(name)
        self.setContentLayout(self.widget_layout)

    def remove_source_from_combo_box(self, source_info):
        item_idx = self.sources_combo_box.findText(source_info.root_folder, Qt.MatchExactly)
        self.sources_combo_box.removeItem(item_idx)