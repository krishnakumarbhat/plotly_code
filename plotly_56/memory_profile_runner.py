#!/usr/bin/env python3
"""
Memory profiling runner for InteractivePlot

This script provides a wrapper to profile memory usage when running 
InteractivePlot data processing. It includes multiple profiling options.

Usage:
    1. From virtual environment: python -m memory_profiler memory_profile_runner.py
    2. For mprof time series: mprof run memory_profile_runner.py && mprof plot
"""
import os
import sys
import argparse
from memory_profiler import profile as mp_profile
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, 
format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger('memory_profiler')

def parse_args():
    parser = argparse.ArgumentParser(description='Profile memory usage in InteractivePlot')
    parser.add_argument('--stream', type=str, default='DETECTION_STREAM',
                        help='Stream type to process')
    parser.add_argument('--sensor', type=str, default='sensor1',
                        help='Sensor to process')
    parser.add_argument('--input-file', type=str, required=False,
                        help='Path to input HDF5 file')
    parser.add_argument('--output-file', type=str, required=False,
                        help='Path to output HDF5 file')
    parser.add_argument('--method', type=str, choices=['decorator', 'line', 'mprof'], 
                        default='decorator',
                        help='Profiling method to use')
    return parser.parse_args()

@mp_profile
def run_interactive_plot(input_file, output_file, stream_name, sensor):
    try:
        # Import data storage classes
        # Adjust these imports based on your actual code structure
        from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
        from InteractivePlot.d_business_layer.data_prep import DataPrep

        logger.info(f"Loading input data from {input_file}")
        # Create your input and output data objects
        # Example (replace with your actual initialization code):
        input_data = DataModelStorage()
        output_data = DataModelStorage()
        
        # Load your data here
        # input_data.load_from_file(input_file)
        # output_data.load_from_file(output_file)
        
        # Profile DataPrep instantiation and plot generation
        logger.info(f"Creating DataPrep object for {stream_name}")
        html_name = f"memory_profile_{stream_name}"
        output_dir = "html_profile"
        os.makedirs(output_dir, exist_ok=True)
        
        # Create the DataPrep object (this will run generate_plots automatically)
        data_prep = DataPrep(
            input_data=input_data,
            output_data=output_data,
            html_name=html_name,
            sensor=sensor,
            stream_name=stream_name,
            output_dir=output_dir
        )
        
        logger.info(f"DataPrep processing complete. HTML output in {output_dir}/{html_name}.html")
        
    except Exception as e:
        logger.error(f"Error during profiling: {str(e)}")
        import traceback
        traceback.print_exc()

def profile_line_by_line(input_file, output_file, stream_name, sensor):
    """
    Run with line-by-line profiling
    """
    from memory_profiler import LineProfiler

    # Import needed classes
    from InteractivePlot.d_business_layer.data_prep import DataPrep
    from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
    
    # Create profile object
    profile = LineProfiler()
    
    # Create data objects
    input_data = DataModelStorage()
    output_data = DataModelStorage()
    
    # Add functions to profile
    profile.add_function(DataPrep.generate_plots)
    profile.add_function(DataPrep._process_signal_plot)
    profile.add_function(DataPrep._get_data_cached)
    
    # Wrap the function call
    def wrapper():
        data_prep = DataPrep(
            input_data=input_data,
            output_data=output_data,
            html_name=f"lineprofile_{stream_name}",
            sensor=sensor,
            stream_name=stream_name,
            output_dir="html_profile"
        )
    
    # Run the profiler
    profile.runctx('wrapper()', globals(), locals())
    
    # Print stats
    profile.print_stats()

def main():
    """Main entry point for profiling"""
    args = parse_args()
    
    logger.info(f"Starting memory profiling with method: {args.method}")
    
    # Check if we're running from the virtual environment
    if not sys.prefix.endswith('venv_profiler'):
        logger.warning("Not running from venv_profiler! Profiling might not work correctly.")
        logger.warning("Run 'source venv_profiler/bin/activate' first.")
    
    # Choose profiling method
    if args.method == 'decorator':
        # Function-level profiling with decorators
        run_interactive_plot(args.input_file, args.output_file, args.stream, args.sensor)
    
    elif args.method == 'line':
        # Line-by-line profiling
        profile_line_by_line(args.input_file, args.output_file, args.stream, args.sensor)
    
    elif args.method == 'mprof':
        # For mprof, just run normally - mprof handles the monitoring
        logger.info("Running with mprof time-series profiling")
        logger.info("After this completes, run 'mprof plot' to see the results")
        run_interactive_plot(args.input_file, args.output_file, args.stream, args.sensor)
    
    logger.info("Memory profiling complete")

if __name__ == "__main__":
    main()
