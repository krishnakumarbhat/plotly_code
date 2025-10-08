# coding=utf-8
"""
Main Rt DVL Extractor Director
"""
from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Interfaces.IExtractor import IExtractor
from aspe.extractors.ReferenceExtractor.RtRange.AdditionalFunctionalities.RtCrossDataSetPostExtraction import (
    RtCrossDataSetPostExtraction,
)
from aspe.extractors.ReferenceExtractor.RtRange.AdditionalFunctionalities.RtDataSetSynch import RtDataSetSynch
from aspe.extractors.ReferenceExtractor.RtRange.Builders.DVL.RtDVLHostBuilder import RtDVLHostBuilder
from aspe.extractors.ReferenceExtractor.RtRange.Builders.DVL.RtDVLMetaDataBuilder import RtDVLMetaDataBuilder
from aspe.extractors.ReferenceExtractor.RtRange.Builders.DVL.RtDVLObjectsBuilder import RtDVLObjectsBuilder
from aspe.extractors.ReferenceExtractor.RtRange.rt_3000_msg_sets_config import rt_3000_message_sets_config_ch_4
from aspe.utilities.DvlSupport import extract_message_sets


class RtDVLExtractor(IExtractor):
    """
    Director Class for extracting rt range data.
    """
    def __init__(self, *, msg_sets_config=None, f_extract_raw_signals=False, f_completed_messages_only=True):
        super().__init__()
        if msg_sets_config is None:
            self._msg_set_config = rt_3000_message_sets_config_ch_4  # default message sets configuration
        else:
            self._msg_set_config = msg_sets_config
        self._f_extract_raw_signals = f_extract_raw_signals
        self._f_completed_messages_only = f_completed_messages_only

    def extract_data(self, parsed_data):
        """
        Creates RtExtractedData and extracting rt data
        :return: RtExtractedData with extracted reference data
        """
        extracted = ExtractedData()

        extracted.metadata = self._build_metadata(parsed_data)
        msg_sets = extract_message_sets(parsed_data['parsed_data'], self._msg_set_config)
        host_raw = self._build_host(msg_sets)
        objects_raw = self._build_objects(msg_sets)

        rt_synch = RtDataSetSynch()
        rt_host_synch, rt_object_synch = rt_synch.synch(master=host_raw, slave=objects_raw)

        if rt_host_synch.f_slave_to_master_synch and rt_object_synch.f_slave_to_master_synch:
            rt_postextract = RtCrossDataSetPostExtraction()
            rt_host_dataset, rt_object_dataset = rt_postextract.extract(host=rt_host_synch, objects=rt_object_synch)
        else:
            rt_host_dataset = None
            rt_object_dataset = None

        extracted.host = rt_host_dataset
        extracted.objects = rt_object_dataset

        return extracted

    def _build_metadata(self, parsed_data):
        metadata_builder = RtDVLMetaDataBuilder(parsed_data)
        return metadata_builder.build()

    def _build_host(self, parsed_data):
        rt_host_builder = RtDVLHostBuilder(parsed_data, f_extract_raw_signals=self._f_extract_raw_signals,
                                                        f_completed_messages_only=self._f_completed_messages_only)
        return rt_host_builder.build()

    def _build_objects(self, parsed_data):
        rt_objects_builder = RtDVLObjectsBuilder(parsed_data, f_extract_raw_signals=self._f_extract_raw_signals,
                                                              f_completed_messages_only=self._f_completed_messages_only)
        return rt_objects_builder.build()

    @staticmethod
    def _get_extractor_name():
        return 'RtDVL'
