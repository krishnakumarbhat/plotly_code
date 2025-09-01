import sys
import time
import threading
import itertools
import os
import multiprocessing as mp
from InteractivePlot.a_config_layer.xml_config_parser import XmlConfigParser
from InteractivePlot.a_config_layer.json_parser_factory import JSONParserFactory
from InteractivePlot.b_persistence_layer.hdf_processor_factory import HdfProcessorFactory
import concurrent.futures

class MainProcessor:
    def __init__(self, xml_file=None, json_file=None, output_dir=None, num_processes=None):
        self.xml_file = xml_file
        self.json_file = json_file
        self.output_dir = output_dir or "html"  # Default to "html" if not provided
        
        # Set number of processes for multiprocessing
        if num_processes is not None:
            try:
                self.num_processes = int(num_processes)
                if self.num_processes <= 0:
                    print(f"Warning: Invalid number of processes ({num_processes}), using system default")
                    self.num_processes = min(os.cpu_count() or 4, 8)  # Default to min(CPU count, 8)
            except ValueError:
                print(f"Warning: Invalid number of processes ({num_processes}), using system default")
                self.num_processes = min(os.cpu_count() or 4, 8)  # Default to min(CPU count, 8)
        else:
            self.num_processes = min(os.cpu_count() or 4, 8)  # Default to min(CPU count, 8)
        
        # Set the multiprocessing environment variable for child processes
        os.environ['INTERACTIVE_PLOT_MP_PROCESSES'] = str(self.num_processes)

    def run(self):
        # Start timing the full execution
        start_time_overall = time.time()
        
        # Print system information
        print(f"\nSystem information:")
        print(f"  CPU cores: {os.cpu_count() or 'Unknown'}")
        print(f"  Using {self.num_processes} processes for parallel processing")
        
        # Start the loading animation
        stop_animation = threading.Event()
        animation_thread = threading.Thread(target=self.loading_animation, args=(stop_animation,))
        animation_thread.daemon = False
        animation_thread.start()
        
        try:
            # Phase 1: Parsing configuration
            print("\nParsing configuration files...")
            start_time_parsing = time.time()
            
            # Parse configuration files in one go
            config_parser = XmlConfigParser(self.xml_file)
            hdf_file_type = config_parser.parse()
            json_parser = JSONParserFactory.create_parser(hdf_file_type, self.json_file)
            input_output_map = json_parser.get_io_map()
            
            end_time_parsing = time.time()
            parsing_time = end_time_parsing - start_time_parsing
            print(f"Parsing time: {parsing_time:.2f} seconds")
            
            # Phase 2 & 3: Data preparation and report generation combined
            print("\nProcessing data and generating reports...")
            start_time_processing = time.time()
            
            # Create factory only once with the number of processes
            factory = HdfProcessorFactory(input_output_map, hdf_file_type, self.output_dir, self.num_processes)
            
            # Print available sensors
            if hasattr(factory, 'available_sensors') and factory.available_sensors:
                print("\nAvailable sensors in HDF:")
                for sensor in factory.available_sensors:
                    print(f"  - {sensor}")
            
            # Process data and generate reports
            factory.process()
            
            end_time_processing = time.time()
            processing_time = end_time_processing - start_time_processing
            print(f"Total processing time: {processing_time:.2f} seconds")
            
            # Overall timing
            end_time_overall = time.time()
            overall_execution_time = end_time_overall - start_time_overall
            print(f"Overall execution time: {overall_execution_time:.2f} seconds")
            
            print("\nReport Generation Completed")
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
            time.sleep(0.1)
        # Clear the spinner line when done
        sys.stdout.write('\r' + ' ' * 20 + '\r')
        sys.stdout.flush()
        
if __name__ == "__main__":
    usage_message = """
    Usage: python script.py <config_file> <input_plot_json_file> [output_directory] [num_processes]

    Arguments:
    config_file           Path to the XML configuration file
    input_plot_json_file  Path to the JSON input file
    output_directory      (Optional) Directory to save HTML reports (default: html)
    num_processes         (Optional) Number of processes to use for parallel processing
                            (default: automatic based on system CPU cores)
    """

    if len(sys.argv) < 3 or len(sys.argv) > 5:
        print(usage_message)
        sys.exit(1)

    config_file = sys.argv[1]
    input_plot_json_file = sys.argv[2]
    output_directory = None
    num_processes = None
    
    # Parse optional arguments
    if len(sys.argv) >= 4:
        output_directory = sys.argv[3]
    if len(sys.argv) >= 5:
        num_processes = sys.argv[4]

    processor = MainProcessor(config_file, input_plot_json_file, output_directory, num_processes)
    processor.run()
