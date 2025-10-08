# coding=utf-8
"""
Interface for F360 MUDP objects builders
"""
from abc import abstractmethod

from aspe.extractors.F360.DataSets.F360Objects import F360Objects
from aspe.extractors.Mudp.IMudpBuilder import IMudpBuilder


class IF360MudpObjectsBuilder(IMudpBuilder):
    """
    Objects builder for F360 MUDP data.
    Version-specific builders should inherit from this class (e.g F360MudpObjectsBuilderV15).
    """
    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)
        self.data_set = F360Objects()

    @abstractmethod
    def build(self):
        """
        Extracts signals and properties from parsed data into dataset.
        Version-specific builders (e.g IF360MudpObjectsBuilderV15) should override this method.
        :return: filled (extracted) data set
        """
        return super().build()
