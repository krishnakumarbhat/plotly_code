from dataclasses import dataclass


@dataclass
class DataSourceInfo:
    log_file_path: str
    pickle_path: str
    root_folder: str
    type: str

    def __eq__(self, other):
        is_equal = self.pickle_path == other.pickle_path and \
                   self.root_folder == other.root_folder and \
                   self.type == other.type
        return is_equal  # noqa: RET504
