from a_config_layer.xml_config_parser import XmlConfigParser
from a_config_layer.json_parser_factory import JSONParserFactory
from b_db_layer.hdf_processor_factory import HdfProcessorFactory

class MainProcessor:
    def __init__(self, xml_file=None, json_file=None , per_sensor_json_file=None):
        self.xml_file = xml_file
        self.json_file = json_file
        self.per_sensor_json_file = per_sensor_json_file
        
    def run(self):
        # Use factory to get the appropriate parser
        config_parser = XmlConfigParser(self.xml_file)
        hdf_file_type = config_parser.parse()
        json_parser = JSONParserFactory.create_parser(hdf_file_type, self.json_file, self.per_sensor_json_file)
        input_output_map = json_parser.get_io_map()
        
        factory = HdfProcessorFactory(input_output_map ,hdf_file_type)
        factory.process()
        
        
if __name__ == "__main__":
    processor = MainProcessor(r"ConfigInteractivePlots.xml" , 
                                r"InputsInteractivePlot.json",
                                r"InputsPerSensorInteractivePlot.json")
    processor.run()
