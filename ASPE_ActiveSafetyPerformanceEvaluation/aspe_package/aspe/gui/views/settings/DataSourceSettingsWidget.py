from PyQt5 import QtCore, QtWidgets


class DataSourceSettingsWidget(QtWidgets.QWidget):
    def __init__(self, data_source, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.parent_data_source = data_source
        self.setupUi()
        self.connect_signals()

    def connect_signals(self):
        self.checkBox.stateChanged.connect(self.on_enabled_state_changed)

    def setupUi(self):
        self.setObjectName("data_source_settings_widget")
        self.resize(450, 461)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Maximum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.sizePolicy().hasHeightForWidth())
        self.setSizePolicy(sizePolicy)
        self.setMaximumSize(QtCore.QSize(450, 500))
        self.verticalLayout = QtWidgets.QVBoxLayout(self)
        self.verticalLayout.setObjectName("verticalLayout")
        self.widget = QtWidgets.QWidget(self)
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
        self.checkBox.setChecked(True)
        self.horizontalLayout.addWidget(self.checkBox)
        self.verticalLayout.addWidget(self.widget)
        self.data_sets_tabs_widget = QtWidgets.QTabWidget(self)
        self.data_sets_tabs_widget.setObjectName("tabWidget")

        self.verticalLayout.addWidget(self.data_sets_tabs_widget)
        self.retranslateUi()
        self.data_sets_tabs_widget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(self)

    def retranslateUi(self):
        _translate = QtCore.QCoreApplication.translate
        self.setWindowTitle(_translate("data_source_settings_widget", "Form"))
        self.label.setText(_translate("data_source_settings_widget", "enabled"))

    def add_data_set_settings(self, data_set_settings_widget, name="tab"):
        self.data_sets_tabs_widget.addTab(data_set_settings_widget, name)

    def on_enabled_state_changed(self):
        is_enabled = self.checkBox.isChecked()
        if is_enabled:
            self.parent_data_source.enable()
            self.data_sets_tabs_widget.setEnabled(True)
        else:
            self.parent_data_source.disable()
            self.data_sets_tabs_widget.setEnabled(False)

    def remove(self):
        self.parent_data_source = None
        self.setParent(None)
