# coding=utf-8
"""
File contains extractor's director interface
"""
from abc import ABC, abstractmethod


class IExtractor(ABC):
    """
    This class should be use to create main extractor director 
    """
    def __init__(self, *args, **kwargs):
        """
        Extractor configuration only, parsed data is passed to extract_data method
        :param args: concrete extractor configuration arguments
        :param kwargs: concrete extractor configuration keywords arguments (e.g. flags what to extract)
        """
        pass

    @abstractmethod
    def extract_data(self, parsed_data):
        """
        main extract function. it shall create data builders
        :return: extracted_data
        """
        raise NotImplementedError('abstract method needs to be implemented')

    @staticmethod
    @abstractmethod
    def _get_extractor_name():
        raise NotImplementedError('abstract method needs to be implemented')
