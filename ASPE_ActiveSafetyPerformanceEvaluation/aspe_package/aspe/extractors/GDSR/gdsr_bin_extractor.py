from dataclasses import dataclass

from aspe.extractors.GDSR.builders.gdsr_bin_metadata_builder import GdsrBinMetadataBuilder
from aspe.extractors.GDSR.builders.gdsr_bin_objects_builder import GdsrBinObjectsBuilder
from aspe.extractors.GDSR.builders.gdsr_bin_radar_sensors_builder import GdsrBinRadarSensorsBuilder
from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Interfaces.IExtractor import IExtractor


@dataclass
class GdsrParsedData:
    tracker_output: dict
    radar_params: dict


class GdsrBinExtractor(IExtractor):
    """Handles extraction of data suitable for evaluation from parsed data."""
    def __init__(self, f_extract_raw_signals=True):
        super().__init__()
        self._f_extract_raw_signals = f_extract_raw_signals

    @staticmethod
    def _get_extractor_name():
        return 'GdsrBin'

    def extract_data(self, parsed_data: GdsrParsedData) -> ExtractedData:
        """Extracts data suitable for evaluation from parsed data.

        :param parsed_data: an instance of GdsrParsedData containing the data parsed from GDSR bin files.
        :return: an instance of ExtractedData that contains the extracted signals.
        """
        extracted = ExtractedData()
        extracted.metadata = self._build_metadata(parsed_data.tracker_output)
        extracted.objects = self._build_objects(parsed_data.tracker_output)
        extracted.sensors = self._build_radar_sensors(parsed_data.radar_params)

        return extracted

    def _build_metadata(self, parsed_data: dict):
        metadata_builder = GdsrBinMetadataBuilder(parsed_data)

        return metadata_builder.build()

    def _build_objects(self, parsed_data: dict):
        objects_builder = GdsrBinObjectsBuilder(parsed_data, f_extract_raw_signals=self._f_extract_raw_signals)

        return objects_builder.build()

    def _build_radar_sensors(self, parsed_data: dict):
        sensors_builder = GdsrBinRadarSensorsBuilder(parsed_data, f_extract_raw_signals=self._f_extract_raw_signals)

        return sensors_builder.build()
