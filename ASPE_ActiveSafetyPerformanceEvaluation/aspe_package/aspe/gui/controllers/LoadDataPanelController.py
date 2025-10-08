from pathlib import Path

from aspe.gui.models.AvailableDataSources import AvailableDataSources


class LoadDataPanelController:
    def __init__(self, load_data_panel_view):
        self.load_data_panel_view = load_data_panel_view
        self.main_controller = None
        self.drawers_controller = None
        self.curr_log_path = ""
        self.curr_log_dir = ""
        self.curr_log_name = ""
        self.curr_data_sources = []
        self.available_sources = None

    def on_log_path_changed(self, new_log_path: str):
        if self.curr_log_path != new_log_path and self.is_valid_file_path(new_log_path):
            try:
                self.main_controller.remove_all_data()
                data_sources = self.load_data_sources(new_log_path)
                self.load_data_panel_view.set_data_sources(data_sources)
            except FileNotFoundError:
                self.load_data_panel_view.show_error_about_not_existing_file()

    def load_data_sources(self, log_path):
        if Path.exists(Path(log_path)):
            self.available_sources = AvailableDataSources(log_path)
            return self.available_sources.sources_info
        raise FileNotFoundError

    def is_valid_file_path(self, file_path):
        is_valid = False
        if Path.exists(Path(file_path)):
            extension = Path(file_path).suffix.lower()
            supported_extensions = (".mudp", ".mf4", ".pickle", ".xtrk", ".bin", ".keg")
            if extension in supported_extensions:
                is_valid = True
        return is_valid

    def on_load_data_from_source(self, source_to_load_dict):
        source_info = self.available_sources.get_source_info_using_dict(source_to_load_dict)
        self.main_controller.load_data_using_source_info(source_info)
        return True

    def clear_data_using_source_info_dict(self, source_info_dict):
        source_info = self.available_sources.get_source_info_using_dict(source_info_dict)
        self.main_controller.remove_data(source_info)

    def clear_sources(self):
        self.curr_data_sources = []
        self.drawers_controller.clear_sources()
