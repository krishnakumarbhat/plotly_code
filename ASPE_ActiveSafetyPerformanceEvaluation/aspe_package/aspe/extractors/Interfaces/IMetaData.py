# coding=utf-8
"""
Main MetaData data set interface
"""
from abc import ABC
from typing import Optional


class IMetaData(ABC):
    """
    MetaData dataset interface class
    """
    def __init__(self):
        self.extractor_type: Optional[str] = None
        self.extractor_version: Optional[str] = None
        self.sw_version: Optional[str] = None
        self.hw_version: Optional[str] = None
        self.data_name: Optional[str] = None
        self.data_path: Optional[str] = None
