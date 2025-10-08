# coding=utf-8
"""
Interface for F360 MUDP detections builders
"""
from abc import abstractmethod

from aspe.extractors.F360.DataSets.F360RadarDetections import F360RadarDetections
from aspe.extractors.Mudp.IMudpBuilder import IMudpBuilder


class IF360MudpDetectionsBuilder(IMudpBuilder):
    """
    Detections builder for F360 MUDP data.
    Version-specific builders should inherit from this class (e.g F360MudpDetectionsBuilderV14).
    """
    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data, f_extract_raw_signals=f_extract_raw_signals)
        self.data_set = F360RadarDetections()

    @abstractmethod
    def build(self):
        """
        Extracts signals and properties from parsed data into dataset.
        Version-specific builders (e.g F360MudpDetectionsBuilderV14) should override this method.
        :return: filled (extracted) data set
        """
        return super().build()
