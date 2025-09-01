from a_config_layer.xml_config_parser import XmlConfigParser
from a_config_layer.json_parser_factory import JSONParserFactory
from b_db_layer.hdf_processor_factory import HdfProcessorFactory
from c_business_layer.data_model import DataContainer
from c_business_layer.data_prep import DataPrep
from d_presentation_layer.plot_generator import PlotGenerator
import os
class MainProcessor:
    def __init__(self, xml_file=None, json_file=None):
        self.xml_file = xml_file
        self.json_file = json_file
        self.data_container = DataContainer() 
        
    def run(self):
        config_parser = XmlConfigParser(self.xml_file)
        hdf_file_type = config_parser.parse()

        # Use factory to get the appropriate parser
        json_parser = JSONParserFactory.create_parser(hdf_file_type, self.json_file)
        input_output_map = json_parser.get_io_map()

        factory = HdfProcessorFactory(self.data_container , input_output_map)
        factory.process()
        

        dataprep = DataPrep(self.data_container)
        html_content = dataprep.generate_html_content()
        print(html_content)

        output_directory = "html"
        hdf_addr = "output_filename.html"
        
        # Ensure the output directory exists before saving HTML
        os.makedirs(output_directory, exist_ok=True)
        
        PlotGenerator.save_html(output_directory, hdf_addr, html_content)
        
if __name__ == "__main__":
    processor = MainProcessor(r"C:/Users/ouymc2/Desktop/plotly25/ConfigInteractivePlots.xml" , 
                            #   r"C:/Users/ouymc2/Desktop/plotly25/InputsInteractivePlot.json")
                              r"C:/Users/ouymc2/Desktop/plotly25/InputsPerSensorInteractivePlot.json")
    processor.run()
