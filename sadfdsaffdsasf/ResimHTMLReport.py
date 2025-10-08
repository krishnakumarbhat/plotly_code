import sys
import os
from InteractivePlot.a_config_layer.xml_config_parser import XmlConfigParser
from InteractivePlot.a_config_layer.json_parser_factory import JSONParserFactory
# from InteractivePlot.a_config_layer.config_manager import ConfigManager, Environment, get_config
from InteractivePlot.b_persistence_layer.hdf_processor_factory import HdfProcessorFactory
from InteractivePlot.d_business_layer.utils import time_taken, LoggerSetup
import asyncio
from concurrent.futures import ThreadPoolExecutor

class MainProcessor:
    def __init__(self, xml_file=None, json_file=None, output_dir=None):
        self.xml_file = xml_file
        self.json_file = json_file
        self.output_dir = output_dir or "html"  # Default to "html" if not provided
        
        # Initialize configuration
        # self.config = ConfigManager(config_file, environment)
        # self.output_dir = self.config.get_output_directory(self.output_dir)

        # Set up enhanced logging
        self.logger_setup = LoggerSetup(self.output_dir)
        self.logger = self.logger_setup.logger
        
        # Log configuration summary
        # self.logger.info(f"Application initialized with {environment.value} configuration")
        # self.logger.info(f"Performance optimization: {self.config.should_optimize_performance()}")
        # self.logger.info(f"Max data points: {self.config.get_max_data_points()}")
        
    @time_taken
    async def run_async(self):
        """Async version of run method"""
        
        # Phase 1: Async config parsing
        config_task = asyncio.create_task(self._parse_config_async())
        config_result = await config_task
        
        # Phase 2: Async data processing
        factory = HdfProcessorFactory(
            config_result['input_output_map'], 
            config_result['hdf_file_type'], 
            self.output_dir
        )
        
        await factory.process_async()
        
    @time_taken
    async def _parse_config_async(self):
        """Parse configuration files asynchronously"""
        loop = asyncio.get_event_loop()
        
        with ThreadPoolExecutor(max_workers=2) as executor:
            # Parse XML and JSON concurrently
            xml_future = loop.run_in_executor(
                executor, self._parse_xml_sync
            )
            json_future = loop.run_in_executor(
                executor, self._parse_json_sync, None
            )
            
            xml_result = await xml_future
            # Update json_future with the hdf_file_type from xml_result
            json_future = loop.run_in_executor(
                executor, self._parse_json_sync, xml_result
            )
            
            json_result = await json_future
            
        return {
            'hdf_file_type': xml_result,
            'input_output_map': json_result
        }
        
    def _parse_xml_sync(self):
        """Parse XML configuration file synchronously"""
        self.logger_setup.log_to_both("Parsing XML configuration file...")
        config_parser = XmlConfigParser(self.xml_file)
        hdf_file_type = config_parser.parse()
        self.logger_setup.log_to_file_only(f"XML parsing complete. HDF file type: {hdf_file_type}")
        return hdf_file_type
        
    def _parse_json_sync(self, hdf_file_type=None):
        """Parse JSON input file synchronously"""
        if hdf_file_type is None:
            # If hdf_file_type is not provided, parse XML first
            hdf_file_type = self._parse_xml_sync()
            
        self.logger_setup.log_to_both("Parsing JSON input file...")
        json_parser = JSONParserFactory.create_parser(hdf_file_type, self.json_file)
        input_output_map = json_parser.get_input_output_map()
        self.logger_setup.log_to_file_only("JSON parsing complete.")
        return input_output_map

# Update main execution
if __name__ == "__main__":
    if sys.platform.startswith("win"):
        import multiprocessing as mp

        mp.freeze_support()
    # PyInstaller compatibility: Fix for file paths when running as executable
    if getattr(sys, "frozen", False):
        # If the application is run as a bundle (PyInstaller executable)
        application_path = os.path.dirname(sys.executable)
        os.chdir(application_path)  # Change working directory to executable location
        print(f"Running as PyInstaller bundle. Application path: {application_path}")

    # Check command line arguments
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print(
            "Usage: python script.py <config_file> <input_plot_json_file> [output_directory]"
        )
        sys.exit(1)

    # Handle file paths
    config_file = sys.argv[1]
    input_plot_json_file = sys.argv[2]
    output_directory = sys.argv[3] if len(sys.argv) == 4 else None

    # Check if files exist
    if not os.path.exists(config_file):
        print(f"Error: Config file not found: {config_file}") 
        print(f"Current working directory: {os.getcwd()}")
        sys.exit(1)

    if not os.path.exists(input_plot_json_file):
        print(f"Error: JSON file not found: {input_plot_json_file}")
        print(f"Current working directory: {os.getcwd()}")
        sys.exit(1)

    print(f"Config file: {config_file}")
    print(f"JSON file: {input_plot_json_file}")
    if output_directory:
        print(f"Output directory: {output_directory}")

    processor = MainProcessor(config_file, input_plot_json_file, output_directory)
    
    # Run with asyncio for better performance
    # try:
    asyncio.run(processor.run_async())
    processor.logger_setup.log_to_both("\nReport Generation Completed")
