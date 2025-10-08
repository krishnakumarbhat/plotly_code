# coding=utf-8
"""
Rt-Range DVL MetaData builder
"""
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.ReferenceExtractor.RtRange.DataSets.RtMetaData import RtMetaData


class RtDVLMetaDataBuilder(IBuilder):
    """
    Builder for Mudp F360 metadata
    """
    def __init__(self, parsed_data):
        super().__init__(parsed_data)
        self.metadata = RtMetaData()

    def build(self):
        """
        Metadata build function
        :return: metadata
        """
        self._fill_metadata()
        return self.metadata

    def _fill_metadata(self):
        self.metadata.data_name = 'RtDVLObjectsExtraction'
        self.metadata.data_path = self._parsed_data['path']
        self.metadata.extractor_type = 'Rt'
        self.metadata.extractor_version = '1.0'
        self.metadata.hw_version = 'unknown'
        self.metadata.sw_version = 'unknown'
