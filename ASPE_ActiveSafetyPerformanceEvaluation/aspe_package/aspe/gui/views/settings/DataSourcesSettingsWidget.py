from PyQt5 import QtCore, QtWidgets
from PyQt5.QtCore import Qt


class DataSourcesSettingsWidget(QtWidgets.QWidget):
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
        self.resize(462, 645)
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.groupBox = QtWidgets.QGroupBox(self)
        self.groupBox.setObjectName("groupBox")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.groupBox)
        self.verticalLayout.setObjectName("verticalLayout")
        self.widget = QtWidgets.QWidget(self.groupBox)
        self.widget.setMaximumSize(QtCore.QSize(16777215, 35))
        self.widget.setObjectName("widget")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.widget)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.label = QtWidgets.QLabel(self.widget)
        self.label.setObjectName("label")
        self.horizontalLayout.addWidget(self.label)
        self.sources_combo_box = QtWidgets.QComboBox(self.widget)
        self.sources_combo_box.setObjectName("comboBox")
        self.horizontalLayout.addWidget(self.sources_combo_box)
        self.horizontalLayout.setStretch(0, 1)
        self.horizontalLayout.setStretch(1, 2)
        self.verticalLayout.addWidget(self.widget)
        self.sources_stacked_widget = QtWidgets.QStackedWidget(self.groupBox)
        self.sources_stacked_widget.setObjectName("stackedWidget")

        self.verticalLayout.addWidget(self.sources_stacked_widget)
        self.verticalLayout_2.addWidget(self.groupBox)
        self.retranslateUi()
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.setWindowTitle(_translate("Form", "Form"))
        self.groupBox.setTitle(_translate("Form", "Settings"))
        self.label.setText(_translate("Form", "data source"))

    def add_data_source_settings(self, data_source_setting_widget, name="source"):
        self.sources_stacked_widget.addWidget(data_source_setting_widget)
        self.sources_combo_box.addItem(name)

    def remove_source_from_combo_box(self, source_info):
        item_idx = self.sources_combo_box.findText(source_info.root_folder, Qt.MatchExactly)
        self.sources_combo_box.removeItem(item_idx)