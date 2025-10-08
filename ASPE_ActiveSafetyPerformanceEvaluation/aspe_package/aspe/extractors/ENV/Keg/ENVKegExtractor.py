import numpy as np

from aspe.extractors.ENV.DataSets.ENVExtractedData import ENVExtractedData
from aspe.extractors.ENV.Keg.builders.ENVKegDetectionsBuilder import ENVKegDetectionsBuilder
from aspe.extractors.ENV.Keg.builders.ENVKegHostBuilder import ENVKegHostBuilder
from aspe.extractors.ENV.Keg.builders.ENVKegMetadataBuilder import ENVKegMetadataBuilder
from aspe.extractors.ENV.Keg.builders.ENVKegObjectsBuilder import ENVKegObjectsBuilder
from aspe.extractors.ENV.Keg.builders.ENVKegOccupancyGridBuilder import ENVKegOccupancyGridBuilder
from aspe.extractors.ENV.Keg.builders.ENVKegRadarSensorsBuilder import ENVKegRadarSensorsBuilder
from aspe.extractors.ENV.Keg.builders.ENVKegStationaryGeometriesBuilder import ENVKegStationaryGeometriesBuilder
from aspe.extractors.Interfaces.IExtractor import IExtractor


class ENVKegExtractor(IExtractor):
    """Handles extraction of data suitable for evaluation from parsed data. Extractor returns ENVExtractedData."""

    def __init__(self, f_extract_raw_signals=True):
        super().__init__()
        self._f_extract_raw_signals = f_extract_raw_signals

    def extract_data(self, parsed_data) -> ENVExtractedData:
        """
        Extracts data suitable for evaluation from parsed data.

        :param parsed_data: an instance of ENVParsedData containing the data parsed from ENV bin files.
        :return: an instance of ExtractedData that contains the extracted signals.
        """
        metadata = parsed_data.get('metadata', None)
        # TODO: FZE-400  Add a different handling for cases when multiple keys will be available
        for key in parsed_data['parsed_data']['data'].keys():
            data = parsed_data['parsed_data']['data'][key]
        timestamps = parsed_data['parsed_data']['timestamps']
        extracted = ENVExtractedData()
        extracted.metadata = self._build_metadata(metadata)
        extracted.detections = self._build_data_set(data, timestamps, ENVKegDetectionsBuilder)
        extracted.sensors = self._build_data_set(data, timestamps, ENVKegRadarSensorsBuilder)
        extracted.host = self._build_data_set(data, timestamps, ENVKegHostBuilder)
        extracted.objects = self._build_data_set(data, timestamps, ENVKegObjectsBuilder, 'OBJECTS')
        # TODO FZE-400 implement when objects data will be present in CR logs, should get whole data structure
        extracted.occupancy_grid = self._build_data_set(data, timestamps, ENVKegOccupancyGridBuilder, 'OCG')
        extracted.stationary_geometries = self._build_data_set(data, timestamps, ENVKegStationaryGeometriesBuilder,
                                                               'SG')
        return extracted

    def _build_data_set(self, data: dict, timestamps: np.ndarray, builder_class, key=None):
        data_set = None
        specific_parsed_data = data.get(key, None)
        if specific_parsed_data is not None or key is None:
            builder = builder_class(data, timestamps, f_extract_raw_signals=self._f_extract_raw_signals)
            data_set = builder.build()
        return data_set

    @staticmethod
    def _build_metadata(metadata: dict) -> dict:
        if metadata is not None:
            builder = ENVKegMetadataBuilder(metadata)
            extracted_metadata = builder.build()
        return extracted_metadata

    @staticmethod
    def _get_extractor_name():
        return 'ENVKEG'


# BELOW CODE IS FOR MODULE TESTING
if __name__ == '__main__':
    from aspe.extractors.API.keg import extract_env_from_keg

    p = r"C:\Users\wykdbz\Downloads\logging\SGOutput.keg"
    extracted = extract_env_from_keg(p, force_extract=True)
    print('Extracting test done.')
