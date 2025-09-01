import sys
import time
import logging
import os
from InteractivePlot.a_config_layer.xml_config_parser import XmlConfigParser
from InteractivePlot.a_config_layer.json_parser_factory import JSONParserFactory
from InteractivePlot.b_persistence_layer.hdf_processor_factory import (
    HdfProcessorFactory,
)
import asyncio
from concurrent.futures import ThreadPoolExecutor

class MainProcessor:
    def __init__(self, xml_file=None, json_file=None, output_dir=None):
        self.xml_file = xml_file
        self.json_file = json_file
        self.output_dir = output_dir or "html"  # Default to "html" if not provided

        # Set up logging
        self.setup_logging()

    def setup_logging(self):
        # Create logs directory if it doesn't exist
        os.makedirs(self.output_dir, exist_ok=True)
        logs_file = os.path.join(self.output_dir, "logs.txt")

        # Configure root logger
        self.logger = logging.getLogger()
        self.logger.setLevel(logging.DEBUG)

        # Clear any existing handlers
        for handler in self.logger.handlers[:]:
            self.logger.removeHandler(handler)

        # Create file handler for logs.txt
        file_handler = logging.FileHandler(logs_file, mode="w")
        file_handler.setLevel(logging.DEBUG)
        file_formatter = logging.Formatter("%(asctime)s - %(levelname)s: %(message)s")
        file_handler.setFormatter(file_formatter)

        # Create console handler
        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setLevel(logging.INFO)
        console_formatter = logging.Formatter("%(message)s")
        console_handler.setFormatter(console_formatter)

        # Add handlers to logger
        self.logger.addHandler(file_handler)
        self.logger.addHandler(console_handler)

        logging.info(f"Logging initialized. Log file: {logs_file}")
        logging.info("Interactive html version 1.0")

    def log_to_both(self, message):
        """Log message to both console and logs.txt"""
        self.logger.info(message)

    def log_to_file_only(self, message):
        """Log message only to logs.txt"""
        self.logger.debug(message)

    def log_to_console_only(self, message):
        """Log message only to console"""
        # Temporarily remove file handler, log, then add it back
        file_handler = self.logger.handlers[0]
        self.logger.removeHandler(file_handler)
        self.logger.info(message)
        self.logger.addHandler(file_handler)

    def run(self):
        # Start timing the full execution
        start_time_overall = time.time()

        # Phase 1: Parsing configuration
        logging.info("\nParsing configuration files...")
        start_time_parsing = time.time()

        # Parse configuration files in one go
        config_parser = XmlConfigParser(self.xml_file)
        hdf_file_type = config_parser.parse()
        json_parser = JSONParserFactory.create_parser(hdf_file_type, self.json_file)
        input_output_map = json_parser.get_input_output_map()

        end_time_parsing = time.time()
        parsing_time = end_time_parsing - start_time_parsing
        logging.debug(f"config and json Parsing time: {parsing_time:.2f} seconds")

        # Phase 2 & 3: hdf parsing and data processing
        logging.info("\nProcessing data and generating reports...")
        start_time_processing = time.time()

        # Create factory only once
        factory = HdfProcessorFactory(input_output_map, hdf_file_type, self.output_dir)

        # Process data and generate reports
        factory.process()

        end_time_processing = time.time()
        processing_time = end_time_processing - start_time_processing
        logging.info(
            f"Total hdf parsing and data_prep time: {processing_time:.2f} seconds"
        )

        # Overall timing
        end_time_overall = time.time()
        overall_execution_time = end_time_overall - start_time_overall
        logging.info(f"Overall execution time: {overall_execution_time:.2f} seconds")

        logging.info("\nReport Generation Completed")

    async def run_async(self):
        """Async version of run method"""
        start_time_overall = time.time()
        
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
        
        end_time_overall = time.time()
        logging.info(f"Total async execution time: {end_time_overall - start_time_overall:.2f} seconds")
    
    async def _parse_config_async(self):
        """Parse configuration files asynchronously"""
        loop = asyncio.get_event_loop()
        
        with ThreadPoolExecutor(max_workers=2) as executor:
            # Parse XML and JSON concurrently
            xml_future = loop.run_in_executor(
                executor, self._parse_xml_sync
            )
            json_future = loop.run_in_executor(
                executor, self._parse_json_sync
            )
            
            xml_result, json_result = await asyncio.gather(xml_future, json_future)
            
        return {
            'hdf_file_type': xml_result,
            'input_output_map': json_result
        }

# Update main execution
if __name__ == "__main__":
    import sys
    import os

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
    try:
        asyncio.run(processor.run_async())
    except KeyboardInterrupt:
        logging.info("Process interrupted by user")
    except Exception as e:
        logging.error(f"Async execution failed, falling back to sync: {e}")
        processor.run()  # Fallback to sync version
