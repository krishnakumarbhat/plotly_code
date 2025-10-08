import plotly.graph_objects as go
import pandas as pd
import numpy as np
import importlib
from InteractivePlot.c_data_storage.regex_storage import SIGNAL_PATTERNS
from collections import OrderedDict
from KPI.detection_matching_kpi import KpiDataModel
from InteractivePlot.e_presentation_layer.plotly_visualization import PlotlyCharts
from InteractivePlot.e_presentation_layer.html_generator import HtmlGenerator
from InteractivePlot.d_business_layer.data_retriver import DataRetriever
import multiprocessing as mp
from functools import lru_cache

class DataPrep:
    """
    Prepares data for visualization by generating plots and handling missing data.
    Acts as a business layer between data storage and presentation.
    """
    
    def __init__(self, input_data, output_data, html_name, output_dir=None,stream_name='Radar'):
        """
        Initializes DataPrep with necessary parameters.

        Parameters:
            input_data: Input data storage
            output_data: Output data storage
            html_name: Name for the HTML file
            output_dir: Directory to save HTML reports
        """
        self.input_data = input_data._data_container
        self.output_data = output_data._data_container
        self.signal_to_value_map_in = input_data._signal_to_value
        self.signal_to_value_map_out = output_data._signal_to_value
        self.html_name = html_name
        self.output_dir = output_dir or "html"
        self.stream_name = stream_name
        self._data_cache = {}  # Cache for retrieved data

        # This get KPI plots
        self.kpi_plots = KpiDataModel(input_data, output_data, html_name)

        # Generate plots and organize them
        self.plots_hash = self.generate_plots()

        # Pass plots to HTML generator (presentation layer)
        HtmlGenerator(self.plots_hash, self.kpi_plots, self.html_name, self.output_dir)

    @lru_cache(maxsize=128)
    def _get_data_cached(self, signal_name, keys_tuple):
        """
        Cached version of data retrieval to avoid redundant processing
        
        Parameters:
        - signal_name: Name of the signal to get data for
        - keys_tuple: Tuple of unique keys for caching
        
        Returns:
        - data_records from DataRetriever.get_data
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
        Process a single signal and generate its plots
        
        Parameters:
        - args: Tuple containing (signal_name, signal_config, data_cal, unique_keys_tuple)
        
        Returns:
        - List of generated plots
        """
        signal_name, signal_config, data_cal, unique_keys_tuple = args
        plots = []
        
        # Get data records for this signal using cached method
        data_records = self._get_data_cached(signal_name, unique_keys_tuple)
        
        # Try aliases if no data found
        if data_records == 'no_data_in_hdf' and 'aliases' in signal_config:
            for alias in signal_config['aliases']:
                data_records = self._get_data_cached(alias, unique_keys_tuple)
                if data_records != 'no_data_in_hdf':
                    break
        
        if data_records and data_records != 'no_data_in_hdf':
            # Check if we need to apply any special calculations
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
        Generates HTML content for plots by handling data and delegating scatter plot creation to PlotlyCharts.
        
        This method uses multiprocessing to generate plots in parallel for better performance.

        Returns:
        - plots_hash: A dictionary containing plots organized by parent tabs.
        """
        plots_hash = {}
        plots_hash[self.stream_name] = []
        
        # Get all unique keys from both data containers
        unique_keys = list(OrderedDict.fromkeys(list(self.input_data.keys()) + list(self.output_data.keys())))
        unique_keys_tuple = tuple(unique_keys)  # Convert to tuple for caching
        
        # Import the data calculation module
        try:
            data_cal_module = importlib.import_module("InteractivePlot.d_business_layer.data_cal")
            data_cal = data_cal_module.DataCalculations()
            # Set the stream name for the data calculations
            data_cal.set_stream_name(self.stream_name)
        except (ImportError, AttributeError) as e:
            print(f"Warning: Could not import DataCalculations class from data_cal module: {str(e)}")
            data_cal = None
        
        # Prepare arguments for multiprocessing
        mp_args = []
        for signal_name, signal_config in SIGNAL_PATTERNS.items():
            mp_args.append((signal_name, signal_config, data_cal, unique_keys_tuple))
        
        # Determine the number of processes to use
        num_processors = min(mp.cpu_count(), len(mp_args))
        
        # Use multiprocessing to generate plots in parallel if there are multiple signals
        if len(mp_args) > 1 and num_processors > 1:
            try:
                with mp.Pool(processes=num_processors) as pool:
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
            # Sequential processing for small datasets or when multiprocessing is not beneficial
            for args in mp_args:
                plots_hash[self.stream_name].extend(self._process_signal_plot(args))
        
        return plots_hash
