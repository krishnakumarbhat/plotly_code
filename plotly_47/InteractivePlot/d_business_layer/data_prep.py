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
import concurrent.futures
import itertools

class DataPrep:
    """
    Prepares data for visualization by generating plots and handling missing data.
    Acts as a business layer between data storage and presentation.
    """
    
    def __init__(self, input_data, output_data, html_name, output_dir=None, stream_name='Radar'):
        """
        Initializes DataPrep with necessary parameters.

        Parameters:
            input_data: Input data storage
            output_data: Output data storage
            html_name: Name for the HTML file
            output_dir: Directory to save HTML reports
            stream_name: Name of the data stream
        """
        self.input_data = input_data._data_container
        self.output_data = output_data._data_container
        self.signal_to_value_map_in = input_data._signal_to_value
        self.signal_to_value_map_out = output_data._signal_to_value
        self.html_name = html_name
        self.output_dir = output_dir or "html"
        self.stream_name = stream_name
        
        # Get unique keys from data containers
        self.unique_keys = self._get_unique_keys()
        self.unique_keys_tuple = tuple(self.unique_keys)  # Cache-friendly tuple version
        
        # Import data calculation module
        self.data_cal = self._load_data_calculation_module()

        # This get KPI plots
        self.kpi_plots = KpiDataModel(input_data, output_data, html_name)

        # Generate plots and organize them
        self.plots_hash = self.generate_plots()

        # Pass plots to HTML generator (presentation layer)
        HtmlGenerator(self.plots_hash, self.kpi_plots, self.html_name, self.output_dir)
    
    def _get_unique_keys(self):
        """Get unique keys from both input and output data containers."""
        # Use OrderedDict to maintain order while removing duplicates
        return list(OrderedDict.fromkeys(
            list(self.input_data.keys()) + list(self.output_data.keys())
        ))
    
    def _load_data_calculation_module(self):
        """Load data calculation module with proper error handling."""
        try:
            data_cal_module = importlib.import_module("InteractivePlot.d_business_layer.data_cal")
            data_cal = data_cal_module.DataCalculations()
            # Set the stream name for the data calculations
            data_cal.set_stream_name(self.stream_name)
            return data_cal
        except (ImportError, AttributeError) as e:
            print(f"Warning: Could not import DataCalculations class from data_cal module: {str(e)}")
            return None

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
        - args: Tuple containing (signal_name, signal_config)
        
        Returns:
        - List of generated plots
        """
        signal_name, signal_config = args
        plots = []
        
        # Get data records for this signal using cached method
        data_records = self._get_data_cached(signal_name, self.unique_keys_tuple)
        
        # Try aliases if no data found and aliases exist
        if data_records == 'no_data_in_hdf' and 'aliases' in signal_config:
            for alias in signal_config['aliases']:
                data_records = self._get_data_cached(alias, self.unique_keys_tuple)
                if data_records != 'no_data_in_hdf':
                    break
        
        # Process data if available
        if data_records and data_records != 'no_data_in_hdf':
            for plot_type in signal_config.get('plot_types', []):
                func_name = plot_type
                # Check if the function exists in data_cal and call it if available
                if self.data_cal and hasattr(self.data_cal, func_name):
                    try:
                        fig_id, figure = getattr(self.data_cal, func_name)(signal_name, data_records)
                        if figure:  # Only add if figure was created successfully
                            plots.append(figure)
                    except Exception as e:
                        print(f"Error generating plot '{func_name}' for signal '{signal_name}': {str(e)}")
                else:
                    print(f"Function '{func_name}' not found in data_cal.py")
        
        return plots

    def _chunk_signals(self, mp_args, chunk_size=5):
        """
        Chunk signals into smaller groups for batch processing.
        This helps with memory management in multiprocessing.
        
        Parameters:
        - mp_args: List of arguments to process
        - chunk_size: Size of each chunk
        
        Returns:
        - List of chunks, each containing up to chunk_size arguments
        """
        return [mp_args[i:i + chunk_size] for i in range(0, len(mp_args), chunk_size)]

    def _process_chunk(self, chunk):
        """
        Process a chunk of signals.
        
        Parameters:
        - chunk: List of arguments to process
        
        Returns:
        - List of plots generated from the chunk
        """
        chunk_plots = []
        for args in chunk:
            chunk_plots.extend(self._process_signal_plot(args))
        return chunk_plots

    def generate_plots(self):
        """
        Generates HTML content for plots by handling data and delegating plot creation.
        
        This method uses a concurrent.futures approach for controlled parallelism
        and better memory management.

        Returns:
        - plots_hash: A dictionary containing plots organized by parent tabs.
        """
        plots_hash = {self.stream_name: []}
        
        # Simply process plots sequentially to avoid multiprocessing serialization issues
        for signal_name, signal_config in SIGNAL_PATTERNS.items():
            plots = self._process_signal_plot((signal_name, signal_config))
            plots_hash[self.stream_name].extend(plots)
        
        return plots_hash
