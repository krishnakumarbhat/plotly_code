# coding=utf-8
"""
F360 MUDP metadata builder
"""
from aspe.extractors.F360.DataSets.F360MetaData import F360MetaData
from aspe.extractors.Interfaces.IBuilder import IBuilder


class F360MudpMetaDataBuilder(IBuilder):
    """
    Builder for Mudp F360 metadata
    """
    def __init__(self, parsed_data):
        super().__init__(parsed_data)
        self.meta_data = F360MetaData()

    def build(self):
        """
        Metadata build function
        :return: metadata as data_set
        """
        self._fill_metadata()
        return self.meta_data

    def _fill_metadata(self):
        self.meta_data.data_name = 'F360EstimatedObjectsExtraction'
        self.meta_data.data_path = self._parsed_data['path']
        self.meta_data.extractor_type = 'F360'
        self.meta_data.extractor_version = '1.0'
        self.meta_data.hw_version = 'unknown'
        self.meta_data.sw_version = 'unknown'
