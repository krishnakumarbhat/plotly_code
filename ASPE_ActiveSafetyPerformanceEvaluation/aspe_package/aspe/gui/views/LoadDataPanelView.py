from os import path
from pathlib import Path

from PyQt5.Qt import QFileDialog
from PyQt5.QtWidgets import QMessageBox

from aspe.gui.views.uis.load_data_panel import Ui_LoadDataPanel


class LoadDataPanelView(Ui_LoadDataPanel):
    def __init__(self):
        super().__init__()
        self.setupUi()
        self.connect_signals()
        self.last_dir = "c:\\"
        self.load_data_panel_controller = None
        self.data_sources_table.parent_widget = self
        self.setAcceptDrops(True)
        self.current_log_path = ""

    def connect_signals(self):
        self.open_file_dialog_button.clicked.connect(self.on_load_data_button_click)
        self.log_path_combo_box.lineEdit().returnPressed.connect(self.on_log_path_changed)

    def on_load_data_button_click(self):
        log_path, filter = QFileDialog.getOpenFileName(self,
                                                       "Open file",
                                                       self.last_dir,
                                                       "Log files (*.dvl *.mudp *.xtrk *.mf4 *.bin "
                                                       "*.keg *.pickle *nexus_extracted.pickle)")
        if len(log_path) != 0:
            dir_path, log_name = path.split(log_path)
            self.last_dir = dir_path
            self.log_path_combo_box.setCurrentText(log_path)
            self.on_log_path_changed()

    def on_log_path_changed(self):
        log_path = self.log_path_combo_box.currentText().replace('\"', "").replace("\'", "")
        if Path.is_file(Path(log_path)):
            self.current_log_path = log_path
            self.load_data_panel_controller.on_log_path_changed(self.current_log_path)
        else:
            self.log_path_combo_box.setCurrentText(self.current_log_path)

    def set_data_sources(self, data_sources):
        self.data_sources_table.clear_sources()
        for _row, data_source in data_sources.iterrows():
            self.data_sources_table.add_row(data_source)

    def on_load_data_from_source(self, source_info_dict):
        return self.load_data_panel_controller.on_load_data_from_source(source_info_dict)

    def clear_data_using_source_info_dict(self, source_info_dict):
        self.load_data_panel_controller.clear_data_using_source_info_dict(source_info_dict)

    def dragEnterEvent(self, event):
        if event.mimeData().hasUrls():
            event.accept()
        else:
            event.ignore()

    def dropEvent(self, event):
        for url in event.mimeData().urls():
            url_path = url.toLocalFile()
            if Path.is_file(Path(url_path)):
                self.current_log_path = url_path.replace('\"', "").replace("\'", "")
                self.log_path_combo_box.setCurrentText(self.current_log_path)
                self.on_log_path_changed()

    def clear_sources(self):
        self.data_sources_table.clear_sources()
        self.load_data_panel_controller.clear_sources()

    def show_error_about_not_existing_file(self):
        msg = QMessageBox()
        msg.setIcon(QMessageBox.Critical)
        msg.setText("Log file load error")
        msg.setInformativeText("Given log file path does not exist")
        msg.setWindowTitle("Error")
        msg.exec_()