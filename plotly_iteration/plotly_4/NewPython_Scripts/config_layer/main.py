from xml_config_parser import XmlConfigParser
from hdf_json_parser_factory import HDFJSONParserFactory
from hdf_processor import HdfProcessor

class MainProcessor:
    def __init__(self, xml_file=None, json_file=None):
        self.xml_file = xml_file
        self.json_file = json_file

    def run(self):
        config_parser = XmlConfigParser(self.xml_file)
        hdf_file_type = config_parser.parse()
        print(hdf_file_type)

        # Use factory to get the appropriate parser
        json_parser = HDFJSONParserFactory.create_parser(hdf_file_type, self.json_file)
        input_output_map = json_parser.get_io_map()

        processor = HdfProcessor(input_output_map)
        processor.process()

if __name__ == "__main__":
    processor = MainProcessor(
        xml_file="ConfigInteractivePlots.xml",
        json_file="InputsPerSensorInteractivePlot.json",
    )
    processor.run()
