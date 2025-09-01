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

        # This get KPI plots
        self.kpi_plots = KpiDataModel(input_data, output_data, html_name)

        # Generate plots and organize them
        self.plots_hash = self.generate_plots()

        # Pass plots to HTML generator (presentation layer)
        HtmlGenerator(self.plots_hash, self.kpi_plots, self.html_name, self.output_dir)

    def generate_plots(self):
        """
        Generates HTML content for plots by handling data and delegating scatter plot creation to PlotlyCharts.
        
        This method iterates through input and output data and collects data records.
        The actual plot generation and organization is handled by the PlotlyCharts class.

        Returns:
        - plots_hash: A dictionary containing plots organized by parent tabs.
        """
        plots_hash = {}
        plots_hash[self.stream_name] = []
        
        # Get all unique keys from both data containers
        unique_keys = list(OrderedDict.fromkeys(list(self.input_data.keys()) + list(self.output_data.keys())))
        
        # Initialize a dictionary to hold figures/plots
        plots = {}
        
        # Import the data calculation module
        try:
            data_cal_module = importlib.import_module("InteractivePlot.d_business_layer.data_cal")
            data_cal = data_cal_module.DataCalculations()
            # Set the stream name for the data calculations
            data_cal.set_stream_name(self.stream_name)
        except (ImportError, AttributeError) as e:
            print(f"Warning: Could not import DataCalculations class from data_cal module: {str(e)}")
            data_cal = None
            
        # Process each signal pattern
        for signal_name, signal_config in SIGNAL_PATTERNS.items():
            # Get data records for this signal using direct static method call
            data_records = DataRetriever.get_data(
                signal_name, 
                unique_keys,
                self.input_data,
                self.output_data,
                self.signal_to_value_map_in,
                self.signal_to_value_map_out
            )
            
            # Try aliases if no data found
            if data_records == 'no_data_in_hdf' and 'aliases' in signal_config:
                for alias in signal_config['aliases']:
                    data_records = DataRetriever.get_data(
                        alias, 
                        unique_keys,
                        self.input_data,
                        self.output_data,
                        self.signal_to_value_map_in,
                        self.signal_to_value_map_out
                    )
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
                        plots[fig_id] = figure
                        plots_hash[self.stream_name].append(plots[fig_id])
                    else:
                        print(f"Function '{func_name}' not found in data_cal.py")
        return plots_hash
