"""
aptiv_data_parsers.py
====================================
"""


import os
from abc import ABC, abstractmethod


class DataParser(ABC):
    """
    Initialize parser with parameters needed for file decode.
    (All parsers has same high level interface.)
    """

    @abstractmethod
    def __init__(self, *args, **kwargs):
        """
        Initialize parser with parameters needed for file decode.
        """
        pass

    @abstractmethod
    def parse(self, log_file_path: str):
        """
        This is main data parser function; it converts data to basic human readable form; fills self.decoded
        """
        pass

    @staticmethod
    def _check_file(fname: str):
        if not isinstance(fname, str):
            raise Exception('Incorrect input log_file_path type.')
        if not os.path.isfile(fname):
            raise Exception('The file {} does not exist!'.format(fname))
