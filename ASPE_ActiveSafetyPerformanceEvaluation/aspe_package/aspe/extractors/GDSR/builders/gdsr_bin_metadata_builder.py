from aspe.extractors.GDSR.datasets.gdsr_metadata import GdsrMetadata
from aspe.extractors.Interfaces.IBuilder import IBuilder


class GdsrBinMetadataBuilder(IBuilder):
    def __init__(self, parsed_data):
        super().__init__(parsed_data)
        self.data_set = GdsrMetadata()

    def build(self) -> GdsrMetadata:
        """Extracts metadata from parsed data.

        :return: an instance of GdsrMetadata with extracted signals.
        """
        self.data_set.data_name = "GdsrEstimatedObjectsExtraction"
        self.data_set.data_path = self._parsed_data["path"]
        self.data_set.extractor_type = "GDSR"
        self.data_set.extractor_version = "1.0"
        self.data_set.hw_version = "unknown"
        self.data_set.sw_version = "unknown"

        return self.data_set
