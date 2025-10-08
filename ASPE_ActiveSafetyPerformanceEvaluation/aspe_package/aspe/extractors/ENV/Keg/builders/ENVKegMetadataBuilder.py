from aspe.extractors.ENV.DataSets.ENVMetaData import ENVMetadata
from aspe.extractors.ENV.Keg.builders.ENVKegBuilder import ENVKegBuilder


class ENVKegMetadataBuilder(ENVKegBuilder):
    def __init__(self, parsed_data):
        super().__init__(parsed_data)
        self.data_set = ENVMetadata()

    def build(self) -> ENVMetadata:
        """
        Extracts metadata from parsed data.

        :return: an instance of Envionment Metadata with extracted signals.
        """
        self.data_set.annotations = self._parsed_data.get('annotations', None)
        self.data_set.conversion_history = self._parsed_data.get('conversion_history', None)
        self.data_set.creation_date = self._parsed_data.get('creation_date', None)
        self.data_set.version_component_resim_interface = self._parsed_data.get('version_component_resim_interface',
                                                                                None)
        self.data_set.version_component_resim_serialization = self._parsed_data.get(
            'version_component_resim_serialization', None)
        self.data_set.version_keg = self._parsed_data.get('version_keg', None)

        return self.data_set
