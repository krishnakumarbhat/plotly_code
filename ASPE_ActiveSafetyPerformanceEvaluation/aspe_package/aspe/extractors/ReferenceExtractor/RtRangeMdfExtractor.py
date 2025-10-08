# coding=utf-8
"""
Main Rt MDF Extractor Director
"""
from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Interfaces.IExtractor import IExtractor
from aspe.extractors.ReferenceExtractor.RtRange.AdditionalFunctionalities.RtCrossDataSetPostExtraction import (
    RtCrossDataSetPostExtraction,
)
from aspe.extractors.ReferenceExtractor.RtRange.AdditionalFunctionalities.RtDataSetSynch import RtDataSetSynch
from aspe.extractors.ReferenceExtractor.RtRange.Builders.MDF.RtMDFHostBuilder import RtMDFHostBuilder
from aspe.extractors.ReferenceExtractor.RtRange.Builders.MDF.RtMDFMetaDataBuilder import RtMDFMetaDataBuilder
from aspe.extractors.ReferenceExtractor.RtRange.Builders.MDF.RtMDFObjectsBuilder import RtMDFObjectsBuilder
from aspe.utilities.mdf_support import find_master_msg, flatten_rt_range_parsed_data


class RtRangeMdfExtractor(IExtractor):
    """
    Director Class for extracting rt range data from mdf file.
    """

    def __init__(self, *,
                 f_extract_raw_signals=False,
                 f_completed_messages_only=True,
                 hunter_length=4.0, hunter_width=2.0,
                 hunter_target_instances_shift=None,
                 hunter_rear_axle_to_front_bumper_dist=None):
        super().__init__()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._f_completed_messages_only = f_completed_messages_only
        self._host_length = hunter_length
        self._host_width = hunter_width
        self._hunter_target_instances_shift = hunter_target_instances_shift
        self._hunter_rear_axle_to_front_bumper_dist = hunter_rear_axle_to_front_bumper_dist

    def extract_data(self, parsed_data):
        self.master_host = find_master_msg(parsed_data['data'], 'Host')
        self.master_target = find_master_msg(parsed_data['data'], 'Target')
        extracted = ExtractedData()
        flattened_data = flatten_rt_range_parsed_data(parsed_data['data'])
        extracted.metadata = self._build_metadata(flattened_data)
        host_raw = self._build_host(flattened_data)
        objects_raw = self._build_objects(flattened_data)
        rt_synch = RtDataSetSynch(time_instances_slave_compensation=self._hunter_target_instances_shift)
        rt_host_synch, rt_object_synch = rt_synch.synch(master=host_raw, slave=objects_raw)

        rt_postextract = RtCrossDataSetPostExtraction()
        rt_host_synch, rt_object_synch = rt_postextract.extract(host=rt_host_synch, objects=rt_object_synch)

        extracted.host = rt_host_synch
        extracted.objects = rt_object_synch

        return extracted

    def _build_metadata(self, parsed_data):
        metadata_builder = RtMDFMetaDataBuilder(parsed_data)
        return metadata_builder.build()

    def _build_host(self, parsed_data):
        rt_host_builder = RtMDFHostBuilder(parsed_data, self.master_host,
                                           f_extract_raw_signals=self._f_extract_raw_signals,
                                           host_length=self._host_length, host_width=self._host_width,
                                           f_completed_messages_only=self._f_completed_messages_only,
                                           rear_axle_to_front_bumper_dist=self._hunter_rear_axle_to_front_bumper_dist)
        return rt_host_builder.build()

    def _build_objects(self, parsed_data):
        rt_objects_builder = RtMDFObjectsBuilder(parsed_data, self.master_target,
                                                 f_extract_raw_signals=self._f_extract_raw_signals,
                                                 f_completed_messages_only=self._f_completed_messages_only)
        return rt_objects_builder.build()

    @staticmethod
    def _get_extractor_name():
        return 'rt_range_mdf_extractor'
