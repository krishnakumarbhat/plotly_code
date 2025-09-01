import plotly.graph_objects as go
import importlib
from InteractivePlot.c_data_storage.regex_storage import Gen7V1_v2, sequance_of_plot
from collections import OrderedDict
from KPI.a_business_layer.detection_matching_kpi import KpiDataModel
from InteractivePlot.e_presentation_layer.html_generator import HtmlGenerator
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
import multiprocessing as mp

import sys
import logging
import time
import sys
import gc
# Import for memory profiling
from memory_profiler import profile

class DataPrep:
    """
    Prepares data for visualization by generating plots and handling plot data.
    Acts as a business layer between data storage and presentation.
    
    This class is responsible for:
    1. Retrieving data from storage layers (input and output)
    2. Processing and transforming data for visualization
    3. Generating appropriate plot types based on signal configuration
    4. Implementing multiprocessing for efficient data processing
    5. Handling KPI calculations.
    6. Passing visualization data to the presentation layer
    """
    
    def __init__(self, input_data, output_data, html_name,sensor,stream_name, output_dir=None,generate_html=True):
        """
        Initializes DataPrep with necessary parameters.

        Parameters:
            input_data: Input data storage containing signal data
            output_data: Output data storage containing processed signal data
            html_name: Name for the HTML file to be generated
            output_dir: Directory to save HTML reports (defaults to "html")
            stream_name: Type of data stream being processed (e.g., 'Radar', 'DETECTION_STREAM')
            generate_html: Whether to generate HTML file or just return plot data
        """
        # Print memory sizes of input and output data objects
        def get_size_mb(obj):
            # Force garbage collection to get more accurate memory usage
            gc.collect()
            return sys.getsizeof(obj) / (1024 * 1024)
            
        def get_container_size_mb(container):
            if hasattr(container, '_data_container'):
                # Get basic object size
                basic_size = sys.getsizeof(container)
                # Get size of data container
                data_container_size = sys.getsizeof(container._data_container)
                # Estimate size of nested data
                nested_size = 0
                for key, value in container._data_container.items():
                    nested_size += sys.getsizeof(key) + sys.getsizeof(value)
                    # Sample some nested items to estimate deeper levels
                    if value and isinstance(value, list) and len(value) > 0:
                        sample_size = min(10, len(value))
                        nested_size += sum(sys.getsizeof(v) for v in value[:sample_size]) * (len(value) / sample_size)
                return (basic_size + data_container_size + nested_size) / (1024 * 1024)
            return get_size_mb(container)
            
        print(f"\n{'='*50}\nMEMORY USAGE ANALYSIS - {stream_name}\n{'='*50}")
        print(f"Input data object size: {get_size_mb(input_data):.2f} MB")
        print(f"Output data object size: {get_size_mb(output_data):.2f} MB")
        print(f"Input data container estimated size: {get_container_size_mb(input_data):.2f} MB")
        print(f"Output data container estimated size: {get_container_size_mb(output_data):.2f} MB")
        print(f"{'='*50}")
        self.input_data = input_data
        input_data =None #cleaning the memory
        self.output_data = output_data
        output_data =None #cleaning the memory

        self.html_name = html_name
        self.output_dir = output_dir or "html"
        self.sensor = sensor
        self.stream_name = stream_name # Temporary storage for plot data

        # Initialize KPI plots for detection streams
        self.kpi_plots = {}
        if self.stream_name == 'DETECTION_STREAM':
            self.kpi_plots = KpiDataModel(
                self.input_data , 
                self.output_data ,

            ).plots
        a = time.time()
        self.plots_hash = self.generate_plots()
        b = time.time()
        print(f"time taken by genrateplot as whole of {b-a} in {self.stream_name}")

        # Pass plots to HTML generator if requested
        if generate_html:
            HtmlGenerator(
                self.plots_hash, 
                self.kpi_plots, 
                self.html_name, 
                self.output_dir
            )
        
        # Clean data storage after HTML generation
        self.clean_data_storage()

    def clean_data_storage(self):
        """
        Clean data storage to free memory after HTML generation
        """
        self.data_storage.clear()

    
    @profile
    def _get_data_cached(self, signal_name, keys_tuple):
        """
        Cached version of data retrieval to avoid redundant processing.
        Uses lru_cache for performance optimization when the same data 
        is requested multiple times.
        
        Parameters:
        - signal_name: Name of the signal to get data for
        - keys_tuple: Tuple of unique keys for caching (immutable for cache)
        
        Returns:
        - Tuple (data_records, data_dict) from DataModelStorage.get_data or ('no_data_in_hdf', {}) if not found
        """
        # Convert tuple back to list for processing
        unique_keys = list(keys_tuple)
        
        return DataModelStorage.get_data(
            self.input_data,
            self.output_data,
            signal_name, 
            unique_keys,

        )

    def _prepare_data_dict(self, unique_keys):
        """
        Prepare data dictionary in the format {scanindex: [[inputrow], [outputrow]], ...}
        
        Parameters:
        - unique_keys: List of scan indices to process
        
        Returns:
        - Dictionary mapping scan indices to input and output rows
        """
        data_dict = {}
        for scan_idx in unique_keys:
            input_row = self.input_data._data_container.get(scan_idx, [])
            output_row = self.output_data._data_container.get(scan_idx, [])
            data_dict[scan_idx] = [input_row, output_row]
        return data_dict

    @profile
    def _process_signal_plot(self, args):
        """
        Process a single signal and generate its plots.
        This method is designed to be used with multiprocessing for parallel execution.
        
        Parameters:
        - args: Tuple containing (signal_name, signal_config, data_cal, unique_keys_tuple, data_dict)
          * signal_name: Name of the signal to process
          * signal_config: Configuration for this signal from Gen7V1_v2
          * data_cal: DataCalculations instance for special calculations
          * unique_keys_tuple: Tuple of unique keys for caching
          * data_dict: Dictionary mapping scan indices to input and output rows
        
        Returns:
        - Dictionary mapping plot_type to figure objects
        """
        signal_name, signal_config, data_cal, unique_keys_tuple, user_data_dict = args
        plot_data_dict = {}
        
        # Get the complete signal name from the 'call' field if available
        complete_signal_name = signal_name
        if 'call' in signal_config and len(signal_config['call']) > 0:
            complete_signal_name = signal_config['call'][0]
            
        # Get data records for this signal using cached method
        data_records, data_dict = self._get_data_cached(signal_name, unique_keys_tuple)
            
        # Merge user-provided data_dict with returned data_dict if needed
        if not data_dict:
            data_dict = user_data_dict
        
        # Try aliases if no data found (fallback mechanism)
        if data_records == 'no_data_in_hdf' and 'aliases' in signal_config:
            for alias in signal_config['aliases']:
                data_records, alias_data_dict = self._get_data_cached(alias, unique_keys_tuple)
                if data_records != 'no_data_in_hdf':
                    # If we got valid data from an alias, use its data_dict if available
                    if alias_data_dict:
                        data_dict = alias_data_dict
                    break
        
        if data_records and data_records != 'no_data_in_hdf':
            # Process each plot type for this signal
            for plot_type in signal_config.get('plot_types', []):
                func_name = plot_type
                # Check if the function exists in data_cal.py
                if data_cal and hasattr(data_cal, func_name):
                    # Call the function with complete signal name, data_records, and data_dict
                    fig_id, figure = getattr(data_cal, func_name)(complete_signal_name, data_records, data_dict)
                    if figure:  # Only add if figure was created successfully
                        plot_key = f"{complete_signal_name}:{plot_type}"
                        plot_data_dict[plot_key] = figure
                else:
                    # Route function not found messages to logs file
                    logging.debug(f"Function '{func_name}' not found in data_cal.py")
        
        return plot_data_dict

    @profile
    def generate_plots(self):
        """
        Generates HTML content for plots by handling data and delegating plot creation.
        
        This method:
        1. Identifies unique keys from both input and output data
        2. Prepares data dictionary for processing
        3. Processes signals in parallel with multiprocessing
        4. Organizes plots by stream type in a dictionary following the sequence
        
        Returns:
        - plots_hash: A dictionary containing plots organized by stream type
        """
        plots_hash = {}
        signal_plot_dict = {}  # {streamname: {plotname: signalname, ...}}
        
        # Get all unique keys from both data containers (deduplication)
        unique_keys = list(OrderedDict.fromkeys(list(self.input_data._data_container.keys()) + list(self.output_data._data_container.keys())))
        unique_keys_tuple = tuple(unique_keys)  # Convert to tuple for caching
        
        # Prepare data dictionary
        data_dict = self._prepare_data_dict(unique_keys)
        
        # Import the data calculation module for specialized calculations
        try:
            data_cal_module = importlib.import_module("InteractivePlot.d_business_layer.data_cal")
            data_cal = data_cal_module.DataCalculations()
            # Set the stream name for the data calculations (context)
            data_cal.set_stream_name(self.stream_name)
        except (ImportError, AttributeError) as e:
            print(f"Warning: Could not import DataCalculations class from data_cal module: {str(e)}")
            data_cal = None
        
        # Prepare arguments for multiprocessing (one job per signal)
        mp_args = []
        for signal_name, signal_config in Gen7V1_v2.items():
            mp_args.append((signal_name, signal_config, data_cal, unique_keys_tuple, data_dict))
            
            # Create mapping entries for signal plot dictionary
            if self.stream_name not in signal_plot_dict:
                signal_plot_dict[self.stream_name] = {}
            
            # Get complete signal name from config
            complete_signal_name = signal_name
            if 'call' in signal_config and len(signal_config['call']) > 0:
                complete_signal_name = signal_config['call'][0]
                
            # Add mapping for each plot type
            for plot_type in signal_config.get('plot_types', []):
                plot_key = f"{complete_signal_name}:{plot_type}"
                signal_plot_dict[self.stream_name][plot_key] = signal_name
        
        # Determine the optimal number of processes to use
        num_processors = min(mp.cpu_count(), len(mp_args))
        
        # Check if running as PyInstaller frozen executable
        is_frozen = getattr(sys, 'frozen', False)
        
        # Use multiprocessing to generate plots in parallel
        signal_plot_data = {}
        if len(mp_args) > 1 and num_processors > 1 and not is_frozen:
            try:
                # Use spawn method which is more compatible with PyInstaller
                mp_context = mp.get_context('spawn')
                with mp_context.Pool(processes=num_processors) as pool:
                    results = pool.map(self._process_signal_plot, mp_args)
                    
                # Combine all dictionaries from results
                for result_dict in results:
                    signal_plot_data.update(result_dict)
            except Exception as e:
                print(f"Error in multiprocessing: {str(e)}. Falling back to sequential processing.")
                # Fall back to sequential processing if multiprocessing fails
                for args in mp_args:
                    result_dict = self._process_signal_plot(args)
                    signal_plot_data.update(result_dict)
        else:
            # Sequential processing for small datasets or when multiprocessing is not beneficial
            print("Using sequential processing for plot generation")
            for args in mp_args:
                result_dict = self._process_signal_plot(args)
                signal_plot_data.update(result_dict)
        
        # Initialize the plots_hash
        plots_hash[self.stream_name] = []
        
        # Append plots according to the sequence
        for plot_key in sequance_of_plot:
            if plot_key in signal_plot_data:
                parts = plot_key.split(":")
                if len(parts) > 1:
                    plot_type = parts[1]
                    if plot_type == 'histogram_with_count':
                        key = f"{self.stream_name}_histogram"
                        if key not in plots_hash:
                            plots_hash[key] = []
                        plots_hash[key].append(signal_plot_data[plot_key])
                    elif plot_type == 'scatter_with_in_out':
                        key = f"{self.stream_name}_scatter"
                        if key not in plots_hash:
                            plots_hash[key] = []
                        plots_hash[key].append(signal_plot_data[plot_key])
                        
        # # Add any plots not in the sequence (to ensure we don't miss any)
        for plot_key, figure in signal_plot_data.items():
            if plot_key not in sequance_of_plot:
                plots_hash[self.stream_name].append(figure)
        
        return plots_hash