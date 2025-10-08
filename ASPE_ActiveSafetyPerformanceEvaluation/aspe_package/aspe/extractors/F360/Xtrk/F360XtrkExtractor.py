# coding=utf-8
"""
F360 Main Extractor from .xtrk files
"""

from aspe.extractors.F360.DataSets.F360XtrkExtractedData import F360XtrkExtractedData
from aspe.extractors.F360.Xtrk.builders.clusters.F360XtrkDetectionsClustersBuilder import (
    F360XtrkDetectionsClustersBuilder,
)
from aspe.extractors.F360.Xtrk.builders.detections.F360XtrkDetectionsBuilder import F360XtrkDetectionsBuilder
from aspe.extractors.F360.Xtrk.builders.host.F360XtrkHostBuilder import F360XtrkHostBuilder
from aspe.extractors.F360.Xtrk.builders.InternalObjects.F360XtrkInternalObjectsBuilder import (
    F360XtrkInternalObjectsBuilder,
)
from aspe.extractors.F360.Xtrk.builders.sensors.F360XtrkSensorsBuilder import F360XtrkSensorsBuilder
from aspe.extractors.F360.Xtrk.builders.static_env.F360StaticEnvironmentBuilder import F360StaticEnvironmentBuilder
from aspe.extractors.F360.Xtrk.builders.tracker_info.F360XtrkTrackerInfoBuilder import F360XtrkTrackerInfoBuilder
from aspe.extractors.Interfaces.IExtractor import IExtractor


class F360XtrkExtractor(IExtractor):
    """
    Class for extracting tracker F360 data.
    Main extraction method. Create and fill data container - F360ExtractedData.
    """

    def __init__(self, *,
                 f_extract_objects=True,
                 f_extract_internal_objects=True,
                 f_extract_sensors=True,
                 f_extract_host=True,
                 f_extract_detections=True,
                 f_extract_clusters=False,
                 f_extract_stat_env=False,
                 f_builders_extract_raw_signals=True):
        super().__init__()
        self._f_extract_objects = f_extract_objects
        self._f_extract_internal_objects = f_extract_internal_objects
        self._f_extract_sensors = f_extract_sensors
        self._f_extract_host = f_extract_host
        self._f_extract_detections = f_extract_detections
        self._f_extract_clusters = f_extract_clusters
        self._f_extract_stat_env = f_extract_stat_env
        self._f_builders_extract_raw_signals = f_builders_extract_raw_signals

    def extract_data(self, parsed_data) -> F360XtrkExtractedData:
        """
        Calls builder to build F360 metadata and F360 data container.
        :return: F360ExtractedData object
        """
        extracted = F360XtrkExtractedData()
        extracted.metadata = self._build_metadata(parsed_data)
        extracted.tracker_info = self._build_tracker_info(parsed_data)
        extracted.internal_objects = self._build_internal_objects(parsed_data)
        extracted.sensors = self._build_sensors(parsed_data)
        extracted.host = self._build_host(parsed_data)
        extracted.detections = self._build_detections(parsed_data)
        extracted.clusters = self._build_clusters(parsed_data)
        extracted.static_environment = self._build_static_env(parsed_data)
        return extracted

    def _build_metadata(self, parsed_data):
        pass

    def _build_detections(self, parsed_data: dict):
        if self._f_extract_detections:
            builder = F360XtrkDetectionsBuilder(parsed_data, f_extract_raw_signals=self._f_builders_extract_raw_signals)
            return builder.build()

    def _build_sensors(self, parsed_data: dict):
        if self._f_extract_sensors:
            builder = F360XtrkSensorsBuilder(parsed_data, f_extract_raw_signals=self._f_builders_extract_raw_signals)
            return builder.build()

    def _build_tracker_info(self, parsed_data: dict):
        builder = F360XtrkTrackerInfoBuilder(parsed_data)
        return builder.build()

    def _build_host(self, parsed_data: dict):
        if self._f_extract_host:
            builder = F360XtrkHostBuilder(parsed_data, f_extract_raw_signals=self._f_builders_extract_raw_signals)
            return builder.build()

    def _build_internal_objects(self, parsed_data: dict):
        if self._f_extract_internal_objects:
            builder = F360XtrkInternalObjectsBuilder(parsed_data,
                                                     f_extract_raw_signals=self._f_builders_extract_raw_signals)
            return builder.build()

    def _build_clusters(self, parsed_data):
        if self._f_extract_clusters:
            builder = F360XtrkDetectionsClustersBuilder(parsed_data,
                                                        f_extract_raw_signals=self._f_builders_extract_raw_signals)
            return builder.build()

    @staticmethod
    def _get_extractor_name():
        return 'F360XTRK'

    def _build_static_env(self, parsed_data):
        if self._f_extract_stat_env:
            builder = F360StaticEnvironmentBuilder(parsed_data,
                                                   f_extract_raw_signals=self._f_builders_extract_raw_signals)
            return builder.build()


if __name__ == '__main__':
    from aspe.parsers.XtrkParser.XtrkParser import XtrkParser

    path = (r"C:\logs\split_astp_test_from_elvira\rRf360t6010309v205p50_debug_p5"
            r"\WDD404_MRR360_20181217_ASTA2GOT_165531_027_rRf360t6010309v205p50_debug_p5.xtrk")

    parser = XtrkParser()
    parsed = parser.parse(log_file_path=path)

    extracted = F360XtrkExtractor(f_builders_extract_raw_signals=True).extract_data(parsed)

    from aspe.utilities.SupportingFunctions import save_to_pkl

    save_to_pkl(extracted, path.replace('.xtrk', '_f360_xtrk_extracted.pickle'))
