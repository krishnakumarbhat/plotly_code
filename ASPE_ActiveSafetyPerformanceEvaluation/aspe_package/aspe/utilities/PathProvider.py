from logging import info
from pathlib import Path
from shutil import copyfile, copytree
from typing import Union


class PathProvider:
    def __init__(self, *,
                 remote_stream_def_path: str,
                 remote_rt_range_dbc_path: str,
                 force_remote: bool = False,
                 local_dir: Union[str, Path] = Path.home() / '.aspe'):
        r"""
        :param remote_stream_def_path: path to remote directory with stream definitions (preferably as raw string)
        :param remote_rt_range_dbc_path: path to RT Range DBC file (preferably as raw string)
        :param force_remote: default is False
            if True, original paths will always be returned,
            if False, files will be downloaded and paths to local copies will be returned
        :param local_dir: optional path to local directory for downloaded files.
            The default path is `~/.aspe` (Linux) or `%userprofile%\.aspe` (Windows), e.g. `C:\Users\abc123\.aspe`.
            If this directory does not exist, it will be created automatically.
        """
        self.remote_stream_def_path = remote_stream_def_path
        self.remote_rt_range_dbc_path = remote_rt_range_dbc_path
        self.force_remote = force_remote

        if not force_remote:
            self.local_dir = Path(local_dir)
            self.logs_dir = self.local_dir / 'logs'
            self.logging_dir = self.local_dir / 'logging'
            self.stream_def_dir = self.local_dir / 'stream_definitions'
            self.rt_range_dbc_path = self.local_dir / 'RtRange3000.dbc'

            for path in self.local_dir, self.logs_dir, self.logging_dir:
                path.mkdir(parents=True, exist_ok=True)

    def get_log_path(self, original_log_path: str) -> str:
        """
        Depending on force_remote flag, returns original input path (when force_remote is True)
        or downloads remote file and returns path to its local copy (when force_remote is False).
        :param original_log_path: remote path to file
        :return: path to log file
        """
        if self.force_remote:
            return original_log_path
        else:
            log_name = Path(original_log_path).name
            local_path = self.logs_dir / log_name
            if not local_path.exists():
                info(f'Downloading {log_name}...')
                copyfile(original_log_path, str(local_path))
            return str(local_path)

    def get_logging_folder_path(self) -> str:
        """
        Returns path to local directory for MultiLogEvaluation logging.
        :return: path to logging directory
        """
        return str(self.logging_dir)

    def get_mudp_stream_def_path(self) -> str:
        """
        Depending on force_remote flag, returns remote path to stream defintions directory (when force_remote is True)
        or downloads all stream definitions and returns path to local directory (when force_remote is False).
        :return: path to stream definitions directory
        """
        if self.force_remote:
            return self.remote_stream_def_path
        else:
            if not self.stream_def_dir.exists():
                info('Downloading stream definitions...')
                copytree(self.remote_stream_def_path, str(self.stream_def_dir))
            return str(self.stream_def_dir)

    def get_rt_range_dbc_path(self) -> str:
        """
        Depending on force_remote flag, returns remote path to RT Range DBC file (when force_remote is True)
        or downloads the file and returns path to its local copy (when force_remote is False).
        :return: path to RT Range DBC file
        """
        if self.force_remote:
            return self.remote_rt_range_dbc_path
        else:
            if not self.rt_range_dbc_path.exists():
                info(f'Downloading {self.rt_range_dbc_path.name}...')
                copyfile(self.remote_rt_range_dbc_path, str(self.rt_range_dbc_path))
            return str(self.rt_range_dbc_path)
