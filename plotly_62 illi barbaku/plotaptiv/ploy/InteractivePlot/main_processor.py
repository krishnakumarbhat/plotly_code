"""
Main processor for the Interactive Plot System.

This module provides the main entry point for the application,
orchestrating the various components.
"""
import time
import os
from typing import Dict, List, Any, Optional

from InteractivePlot.a_config_layer.xml_config_parser import XmlConfigParser
from InteractivePlot.a_config_layer.json_parser_factory import JSONParserFactory
from InteractivePlot.io.hdf_reader import HdfReaderFactory
from InteractivePlot.multiprocessing.parallel_processor import ParallelProcessor
from InteractivePlot.observers.progress_observer import (
    ProgressReporter, ConsoleProgressObserver, ProgressAnimator
)
from InteractivePlot.visualization.html_report_builder import HtmlReportGenerator


class MainProcessor:
    """
    Main processor for the Interactive Plot System.
    
    This class orchestrates the various components of the system,
    implementing a facade pattern for easier client usage.
    """
    
    def __init__(self, xml_file: Optional[str] = None, json_file: Optional[str] = None, output_dir: Optional[str] = None):
        """
        Initialize the main processor.
        
        Args:
            xml_file: Path to the XML configuration file
            json_file: Path to the JSON input file
            output_dir: Path to the output directory
        """
        self.xml_file = xml_file
        self.json_file = json_file
        self.output_dir = output_dir or "html"
        
        # Create progress reporting infrastructure
        self.progress_reporter = ProgressReporter()
        self.progress_observer = ConsoleProgressObserver()
        self.progress_reporter.register_observer(self.progress_observer)
        self.progress_animator = ProgressAnimator(self.progress_reporter)
        
        # Create parallel processor
        self.parallel_processor = ParallelProcessor()
    
    def run(self) -> None:
        """
        Run the main processing pipeline.
        
        This method orchestrates the entire process:
        1. Parse configuration
        2. Process input files
        3. Generate visualizations
        4. Create HTML reports
        """
        # Start timing the full execution
        start_time_overall = time.time()
        
        # Initialize timing variables
        parsing_time = 0.0
        data_preparation_time = 0.0
        report_generation_time = 0.0
        
        # Start the loading animation
        self.progress_animator.start("Processing...")
        
        try:
            # Phase 1: Parsing configuration
            self.progress_reporter.notify_observers(
                progress=0.0,
                message="Parsing configuration files"
            )
            start_time_parsing = time.time()
            
            # Parse XML configuration
            config_parser = XmlConfigParser(self.xml_file)
            hdf_file_type = config_parser.parse()
            
            # Create JSON parser using factory
            json_parser = JSONParserFactory.create_parser(hdf_file_type, self.json_file)
            input_output_map = json_parser.get_io_map()
            
            end_time_parsing = time.time()
            parsing_time = end_time_parsing - start_time_parsing
            self.progress_reporter.notify_observers(
                progress=10.0,
                message=f"Configuration parsing completed in {parsing_time:.2f} seconds"
            )
            
            # Phase 2: Data preparation
            self.progress_reporter.notify_observers(
                progress=10.0,
                message="Preparing data for processing"
            )
            start_time_dataprep = time.time()
            
            # Create HDF reader using factory
            hdf_reader = HdfReaderFactory.create_reader(hdf_file_type, self.progress_reporter)
            
            # Extract available sensors from the first input file
            if input_output_map:
                input_file = next(iter(input_output_map.keys()))
                available_sensors = hdf_reader.read_sensors(input_file)
                
                self.progress_reporter.notify_observers(
                    progress=15.0,
                    message=f"Found {len(available_sensors)} sensors: {', '.join(available_sensors)}"
                )
            else:
                available_sensors = set()
                self.progress_reporter.notify_observers(
                    progress=15.0,
                    message="No input files found in the configuration"
                )
            
            # Phase 3: Process the files
            self.progress_reporter.notify_observers(
                progress=20.0,
                message="Processing input and output files"
            )
            
            # Define a function to process each file pair
            def process_file_pair(input_file, output_file):
                """Process a pair of input and output files."""
                # Read data from input and output files
                input_data = hdf_reader.read_data(input_file)
                output_data = hdf_reader.read_data(output_file)
                
                # Generate HTML name
                input_name = os.path.basename(input_file).split('.')[0]
                output_name = os.path.basename(output_file).split('.')[0]
                
                # Find the unique part of the output name
                unique_part = output_name
                for i in range(1, min(len(input_name), len(output_name))):
                    if input_name.endswith(output_name[:i]) or output_name.startswith(input_name[-i:]):
                        unique_part = output_name[i:]
                        break
                
                # Create HTML filename
                if unique_part and unique_part != output_name:
                    html_name = f"{input_name}_{unique_part}.html"
                else:
                    html_name = f"{input_name}_{output_name}.html"
                
                # Process each sensor
                visualizations = {}
                
                # Define a function to process each sensor
                def process_sensor(sensor, data=None):
                    """Process a single sensor."""
                    sensor_data = {}
                    
                    # Process the sensor data to create visualizations
                    # In a real implementation, this would call a data processor
                    # For now, we'll just create a placeholder visualization
                    import plotly.graph_objects as go
                    fig = go.Figure()
                    fig.add_trace(go.Scatter(x=[1, 2, 3], y=[4, 5, 6], mode='lines', name=f'Sensor {sensor}'))
                    fig.update_layout(title=f'Visualization for Sensor {sensor}')
                    
                    sensor_data[f"Tab for {sensor}"] = [fig]
                    return sensor_data
                
                # Process sensors in parallel
                if available_sensors:
                    sensor_visualizations = self.parallel_processor.process_sensors_parallel(
                        sensors=list(available_sensors),
                        sensor_data={"input": input_data, "output": output_data},
                        process_func=process_sensor,
                        progress_reporter=self.progress_reporter
                    )
                    
                    # Merge sensor visualizations
                    for sensor, viz in sensor_visualizations.items():
                        visualizations.update(viz)
                else:
                    # If no sensors found, create a default visualization
                    default_visualizations = process_sensor("Default")
                    visualizations.update(default_visualizations)
                
                return (visualizations, html_name)
            
            # Process file pairs in parallel
            file_pairs = [(input_file, output_file) for input_file, output_file in input_output_map.items()]
            processed_results = []
            
            for input_file, output_file in file_pairs:
                result = process_file_pair(input_file, output_file)
                processed_results.append(result)
            
            end_time_dataprep = time.time()
            data_preparation_time = end_time_dataprep - start_time_dataprep
            
            self.progress_reporter.notify_observers(
                progress=70.0,
                message=f"Data preparation completed in {data_preparation_time:.2f} seconds"
            )
            
            # Phase 4: Generate HTML reports
            self.progress_reporter.notify_observers(
                progress=70.0,
                message="Generating HTML reports"
            )
            start_time_report = time.time()
            
            # Create HTML report generator
            report_generator = HtmlReportGenerator(self.progress_reporter)
            
            # Define a function to generate a report
            def generate_report(visualizations, html_name):
                """Generate an HTML report."""
                return report_generator.generate_report(
                    visualizations=visualizations,
                    output_file=html_name,
                    output_dir=self.output_dir
                )
            
            # Generate reports in parallel
            report_data = [(viz, html_name, self.output_dir) for viz, html_name in processed_results]
            
            if report_data:
                # Process each report sequentially
                for viz, html_name in processed_results:
                    generate_report(viz, html_name)
            
            end_time_report = time.time()
            report_generation_time = end_time_report - start_time_report
            
            self.progress_reporter.notify_observers(
                progress=100.0,
                message=f"Report generation completed in {report_generation_time:.2f} seconds"
            )
            
            # Overall timing
            end_time_overall = time.time()
            overall_execution_time = end_time_overall - start_time_overall
            
            self.progress_reporter.notify_observers(
                progress=100.0,
                message=f"Overall execution completed in {overall_execution_time:.2f} seconds"
            )
        except Exception as e:
            self.progress_reporter.notify_observers(
                progress=100.0,
                message=f"Error: {e}"
            )
            raise
        finally:
            # Stop the animation when processing is complete
            self.progress_animator.stop("Processing completed")
            
            # Print timing summary
            print("\nExecution Summary:")
            print(f"Parsing time: {parsing_time:.2f} seconds")
            print(f"Data preparation time: {data_preparation_time:.2f} seconds")
            print(f"Report generation time: {report_generation_time:.2f} seconds")
            print(f"Overall execution time: {overall_execution_time:.2f} seconds")
            print("Report Generation Completed") 