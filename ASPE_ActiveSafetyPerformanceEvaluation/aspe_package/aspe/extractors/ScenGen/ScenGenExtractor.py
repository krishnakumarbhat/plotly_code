from aspe.extractors.Interfaces.IExtractor import IExtractor
from aspe.extractors.ScenGen.Builders.Detections.ScenGenDetectionsBuilder import ScenGenDetectionsBuilder
from aspe.extractors.ScenGen.Builders.Host.ScenGenHostBuilder import ScenGenHostBuilder
from aspe.extractors.ScenGen.Builders.RefContra.ScenGenRefCONTRABuilder import ScenGenRefCONTRABuilder
from aspe.extractors.ScenGen.Builders.Sensors.ScenGenSensorsBuilder import ScenGenSensorsBuilder
from aspe.extractors.ScenGen.DataSets.ScenGenExtractedData import ScenGenExtractedData


class ScenGenExtractor(IExtractor):
    def __init__(self, f_extract_raw_signals: bool = True):
        super(ScenGenExtractor, self).__init__()

        self._f_extract_raw_signals = f_extract_raw_signals

    def extract_data(self, parsed_data: dict) -> ScenGenExtractedData:
        """
        Calls builders to build Scenario Generator metadata and Scenario Generator data.

        @param parsed_data: Scenario Generator dictionary parsed from .mat file
        @return: ScenGenExtractedData
        """
        extracted = ScenGenExtractedData()
        extracted.metadata = self._build_metadata(parsed_data)
        extracted.host = self._build_data_set(parsed_data, 'vehicle_data', ScenGenHostBuilder)
        extracted.sensors = self._build_data_set(parsed_data, 'sensors_config', ScenGenSensorsBuilder)
        extracted.detections = self._build_data_set(parsed_data, 'detections', ScenGenDetectionsBuilder)
        extracted.contra_reference = self._build_data_set(parsed_data, 'ref', ScenGenRefCONTRABuilder)

        return extracted

    @staticmethod
    def _build_metadata(parsed_data: dict) -> dict:
        """Extract metadata from parsed .mat file."""
        metadata = dict()

        testcase_data = parsed_data['testcase']

        metadata['scenario_name'] = testcase_data['scenario_name']
        metadata['disturbance_name'] = testcase_data['disturb_name']
        metadata['used_changeset'] = testcase_data['dets_gen_changeset']

        metadata['version'] = parsed_data['__version__']
        metadata['header'] = parsed_data['__header__']

        return metadata

    def _build_data_set(self, parsed_data: dict, dataset_name: str, builder_class) -> object:
        data_set = None
        if parsed_data.get(dataset_name, None) is not None:
            builder = builder_class(parsed_data, self._f_extract_raw_signals)
            data_set = builder.build()
        return data_set

    @staticmethod
    def _get_extractor_name() -> str:
        return 'ScenarioGenerator'


if __name__ == '__main__':
    from aspe.parsers.MatParser.MatParser import MatParser

    path = r"PATH/TO/ScenarioGenerator"

    parser = MatParser()
    parsed = parser.parse(log_file_path=path)

    extractor = ScenGenExtractor()
    extracted = extractor.extract_data(parsed_data=parsed)

    print('Done')
