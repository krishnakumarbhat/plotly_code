import sys
import time
import threading
import itertools
import logging
import os
from datetime import datetime
from InteractivePlot.a_config_layer.xml_config_parser import XmlConfigParser
from InteractivePlot.a_config_layer.json_parser_factory import JSONParserFactory
from InteractivePlot.b_persistence_layer.hdf_processor_factory import HdfProcessorFactory

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
        file_handler = logging.FileHandler(logs_file, mode='w')
        file_handler.setLevel(logging.DEBUG)
        file_formatter = logging.Formatter('%(asctime)s - %(levelname)s: %(message)s')
        file_handler.setFormatter(file_formatter)
        
        # Create console handler
        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setLevel(logging.INFO)
        console_formatter = logging.Formatter('%(message)s')
        console_handler.setFormatter(console_formatter)
        
        # Add handlers to logger
        self.logger.addHandler(file_handler)
        self.logger.addHandler(console_handler)
        
        logging.info(f"Logging initialized. Log file: {logs_file}")

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
        
        # Start the loading animation
        stop_animation = threading.Event()
        animation_thread = threading.Thread(target=self.loading_animation, args=(stop_animation,))
        animation_thread.daemon = True
        animation_thread.start()
        
        try:
            # Phase 1: Parsing configuration
            print("\nParsing configuration files...")
            start_time_parsing = time.time()

            # Parse configuration files in one go
            config_parser = XmlConfigParser(self.xml_file)
            hdf_file_type = config_parser.parse()
            json_parser = JSONParserFactory.create_parser(hdf_file_type, self.json_file)
            input_output_map = json_parser.get_input_output_map()
            
            end_time_parsing = time.time()
            parsing_time = end_time_parsing - start_time_parsing
            logging.info(f"config and json Parsing time: {parsing_time:.2f} seconds")
            
            
            # Phase 2 & 3: hdf parsing and data processing
            logging.info("\nProcessing data and generating reports...")
            start_time_processing = time.time()
            
            # Create factory only once
            factory = HdfProcessorFactory(input_output_map, hdf_file_type, self.output_dir)
            
            # Print available sensors
            if hasattr(factory, 'available_sensors') and factory.available_sensors:
                logging.info("\nAvailable sensors in HDF:")
                for sensor in factory.available_sensors:
                    logging.info(f"  - {sensor}")
            
            # Process data and generate reports
            factory.process()
            
            end_time_processing = time.time()
            processing_time = end_time_processing - start_time_processing
            logging.info(f"Total hdf parsing and data_prep time: {processing_time:.2f} seconds")
            
            # Overall timing
            end_time_overall = time.time()
            overall_execution_time = end_time_overall - start_time_overall
            logging.info(f"Overall execution time: {overall_execution_time:.2f} seconds")
            
            logging.info("\nReport Generation Completed")
        finally:
            # Stop the animation when processing is complete
            stop_animation.set()
            animation_thread.join()
    
    def loading_animation(self, stop_event):
        """Displays a spinner animation in the console while the process is running."""
        spinner = itertools.cycle(['|', '/', '-', '\\'])
        while not stop_event.is_set():
            sys.stdout.write('\rProcessing... ' + next(spinner))
            sys.stdout.flush()
            time.sleep(0.03)
        # Clear the spinner line when done
        sys.stdout.write('\r' + ' ' * 20 + '\r')
        sys.stdout.flush()
        
if __name__ == "__main__":
    import sys
    import os
    
    # PyInstaller compatibility: Fix for file paths when running as executable
    if getattr(sys, 'frozen', False):
        # If the application is run as a bundle (PyInstaller executable)
        application_path = os.path.dirname(sys.executable)
        os.chdir(application_path)  # Change working directory to executable location
        print(f"Running as PyInstaller bundle. Application path: {application_path}")
        
    # Check command line arguments
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print("Usage: python script.py <config_file> <input_plot_json_file> [output_directory]")
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
    processor.run()