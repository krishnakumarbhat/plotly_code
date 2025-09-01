import plotly.graph_objects as go
import importlib
from InteractivePlot.c_data_storage.regex_storage import SIGNAL_PATTERNS
from collections import OrderedDict
from KPI.a_business_layer.detection_matching_kpi import KpiDataModel
from InteractivePlot.e_presentation_layer.plotly_visualization import PlotlyCharts
from InteractivePlot.e_presentation_layer.html_generator import HtmlGenerator
from InteractivePlot.d_business_layer.data_retriver import DataRetriever
import multiprocessing as mp
from functools import lru_cache
import sys

class DataPrep:
    """
    Prepares data for visualization by generating plots and handling missing data.
    Acts as a business layer between data storage and presentation.
    
    This class is responsible for:
    1. Retrieving data from storage layers (input and output)
    2. Processing and transforming data for visualization
    3. Generating appropriate plot types based on signal configuration
    4. Implementing multiprocessing for efficient data processing
    5. Handling KPI calculations when applicable
    6. Passing visualization data to the presentation layer
    """
    
    def __init__(self, input_data, output_data, html_name, output_dir=None, stream_name='Radar'):
        """
        Initializes DataPrep with necessary parameters.

        Parameters:
            input_data: Input data storage containing signal data
            output_data: Output data storage containing processed signal data
            html_name: Name for the HTML file to be generated
            output_dir: Directory to save HTML reports (defaults to "html")
            stream_name: Type of data stream being processed (e.g., 'Radar', 'DETECTION_STREAM')
        """
        self.input_data = input_data._data_container
        self.output_data = output_data._data_container
        self.signal_to_value_map_in = input_data._signal_to_value
        self.signal_to_value_map_out = output_data._signal_to_value
        self.html_name = html_name
        self.output_dir = output_dir or "html"
        self.stream_name = stream_name
        self._data_cache = {}  # Cache for retrieved data to improve performance

        # Initialize KPI plots for detection streams
        self.kpi_plots = {}
        if self.stream_name == 'DETECTION_STREAM':
            self.kpi_plots = KpiDataModel(
                input_data, 
                output_data,
                self.signal_to_value_map_in,
                self.signal_to_value_map_out
            ).plots

        # Generate plots and organize them by stream type
        self.plots_hash = self.generate_plots()

        # Pass plots to HTML generator (presentation layer) for final output
        HtmlGenerator(
            self.plots_hash, 
            self.kpi_plots, 
            self.html_name, 
            self.output_dir
        )

    @lru_cache(maxsize=None)
    def _get_data_cached(self, signal_name, keys_tuple):
        """
        Cached version of data retrieval to avoid redundant processing.
        Uses lru_cache for performance optimization when the same data 
        is requested multiple times.
        
        Parameters:
        - signal_name: Name of the signal to get data for
        - keys_tuple: Tuple of unique keys for caching (immutable for cache)
        
        Returns:
        - data_records from DataRetriever.get_data or 'no_data_in_hdf' if not found
        """
        # Convert tuple back to list for processing
        unique_keys = list(keys_tuple)
        
        return DataRetriever.get_data(
            signal_name, 
            unique_keys,
            self.input_data,
            self.output_data,
            self.signal_to_value_map_in,
            self.signal_to_value_map_out
        )

    def _process_signal_plot(self, args):
        """
        Process a single signal and generate its plots.
        This method is designed to be used with multiprocessing for parallel execution.
        
        Parameters:
        - args: Tuple containing (signal_name, signal_config, data_cal, unique_keys_tuple)
          * signal_name: Name of the signal to process
          * signal_config: Configuration for this signal from SIGNAL_PATTERNS
          * data_cal: DataCalculations instance for special calculations
          * unique_keys_tuple: Tuple of unique keys for caching
        
        Returns:
        - List of generated Plotly figure objects
        """
        signal_name, signal_config, data_cal, unique_keys_tuple = args
        plots = []
        
        # Get data records for this signal using cached method
        data_records = self._get_data_cached(signal_name, unique_keys_tuple)
        
        # Try aliases if no data found (fallback mechanism)
        if data_records == 'no_data_in_hdf' and 'aliases' in signal_config:
            for alias in signal_config['aliases']:
                data_records = self._get_data_cached(alias, unique_keys_tuple)
                if data_records != 'no_data_in_hdf':
                    break
        
        if data_records and data_records != 'no_data_in_hdf':
            # Check if we need to apply any special calculations based on plot types
            for plot_type in signal_config.get('plot_types', []):
                func_name = plot_type
                # Check if the function exists in data_cal.py
                if data_cal and hasattr(data_cal, func_name):
                    # Call the function with signal_name and data_records
                    fig_id, figure = getattr(data_cal, func_name)(signal_name, data_records)
                    if figure:  # Only add if figure was created successfully
                        plots.append(figure)
                else:
                    print(f"Function '{func_name}' not found in data_cal.py")
        
        return plots

    def generate_plots(self):
        """
        Generates HTML content for plots by handling data and delegating plot creation.
        
        This method:
        1. Identifies unique keys from both input and output data
        2. Prepares arguments for multiprocessing
        3. Processes signals in parallel for better performance
        4. Falls back to sequential processing if multiprocessing fails
        5. Organizes plots by stream type in a dictionary
        
        Returns:
        - plots_hash: A dictionary containing plots organized by stream type
        """
        plots_hash = {}
        plots_hash[self.stream_name] = []
        
        # Get all unique keys from both data containers (deduplication)
        unique_keys = list(OrderedDict.fromkeys(list(self.input_data.keys()) + list(self.output_data.keys())))
        unique_keys_tuple = tuple(unique_keys)  # Convert to tuple for caching
        
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
        for signal_name, signal_config in SIGNAL_PATTERNS.items():
            mp_args.append((signal_name, signal_config, data_cal, unique_keys_tuple))
        
        # Determine the optimal number of processes to use
        num_processors = min(mp.cpu_count(), len(mp_args))
        
        # Check if running as PyInstaller frozen executable
        is_frozen = getattr(sys, 'frozen', False)
        
        # Use multiprocessing to generate plots in parallel if there are multiple signals
        # This significantly improves performance for large datasets
        if len(mp_args) > 1 and num_processors > 1 and not is_frozen:
            try:
                # Use spawn method which is more compatible with PyInstaller
                mp_context = mp.get_context('spawn')
                with mp_context.Pool(processes=num_processors) as pool:
                    results = pool.map(self._process_signal_plot, mp_args)
                    
                # Flatten the results list and add to plots_hash
                for result in results:
                    plots_hash[self.stream_name].extend(result)
            except Exception as e:
                print(f"Error in multiprocessing: {str(e)}. Falling back to sequential processing.")
                # Fall back to sequential processing if multiprocessing fails
                for args in mp_args:
                    plots_hash[self.stream_name].extend(self._process_signal_plot(args))
        else:
            # Sequential processing for small datasets, when multiprocessing is not beneficial,
            # or when running as a frozen executable (PyInstaller)
            print("Using sequential processing for plot generation")
            for args in mp_args:
                plots_hash[self.stream_name].extend(self._process_signal_plot(args))
        
        return plots_hash