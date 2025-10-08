from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Interfaces.IExtractor import IExtractor
from aspe.extractors.ReferenceExtractor.dSpace.builders.dspace_bin_metadata_builder import DspaceBinMetadataBuilder
from aspe.extractors.ReferenceExtractor.dSpace.builders.dspace_bin_objects_builder import DspaceBinObjectsBuilder


class DSpaceBinExtractor(IExtractor):
    """Handles extraction of data suitable for evaluation from parsed data."""
    def __init__(self, f_extract_raw_signals=True):
        super().__init__()
        self._f_extract_raw_signals = f_extract_raw_signals

    @staticmethod
    def _get_extractor_name():
        return 'DSpaceBin'

    def extract_data(self, parsed_data) -> ExtractedData:
        """Extracts dSpace reference data suitable for evaluation from parsed data.

        :param parsed_data: data parsed from a container.
        :return: an instance of ExtractedData that contains the extracted signals.
        """
        extracted = ExtractedData()
        extracted.metadata = self._build_metadata(parsed_data)
        extracted.objects = self._build_objects(parsed_data)

        return extracted

    def _build_metadata(self, parsed_data):
        metadata_builder = DspaceBinMetadataBuilder(parsed_data)

        return metadata_builder.build()

    def _build_objects(self, parsed_data):
        objects_builder = DspaceBinObjectsBuilder(parsed_data, f_extract_raw_signals=self._f_extract_raw_signals)

        return objects_builder.build()
