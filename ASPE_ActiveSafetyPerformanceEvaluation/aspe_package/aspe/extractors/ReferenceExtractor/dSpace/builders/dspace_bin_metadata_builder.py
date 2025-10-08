from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.ReferenceExtractor.dSpace.datasets.dspace_metadata import DspaceMetadata


class DspaceBinMetadataBuilder(IBuilder):
    """Builder for dSpace OSI reference/ground-truth data parsed from a bin file."""

    def __init__(self, parsed_data):
        super().__init__(parsed_data)
        self.data_set = DspaceMetadata()

    def build(self) -> DspaceMetadata:
        """Extracts metadata from parsed data.

        :return: an instance of DspaceMetadata with extracted signals.
        """
        self.data_set.data_name = "dSpaceBinMetadataExtraction"
        self.data_set.data_path = self._parsed_data["path"]
        self.data_set.extractor_type = "dSpace"
        self.data_set.extractor_version = "1.0"
        self.data_set.hw_version = "unknown"
        self.data_set.sw_version = "unknown"

        return self.data_set
