import pathlib


class DataModel:
    def __init__(self, extracted, source_info):
        self.name = pathlib.Path(source_info.log_file_path).name
        self.extracted = extracted
        self.source_info = source_info
