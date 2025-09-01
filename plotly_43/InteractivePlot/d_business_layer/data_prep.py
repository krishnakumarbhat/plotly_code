import plotly.graph_objects as go
import pandas as pd
import numpy as np
from InteractivePlot.c_data_storage.regex_storage import SIGNAL_PATTERNS

from KPI.detection_matching_kpi import KpiDataModel
from InteractivePlot.e_presentation_layer.plotly_visualization import PlotlyCharts
from InteractivePlot.e_presentation_layer.html_generator import HtmlGenerator

class DataPrep:
    """
    Prepares data for visualization by generating plots and handling missing data.
    """
    
    def __init__(self, input_data, output_data, html_name, output_dir=None):
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
        self.val_sig_map_in = input_data._value_to_signal
        self.sig_val_map_in = input_data._signal_to_value
        self.val_sig_map_out = output_data._value_to_signal
        self.sig_val_map_out = output_data._signal_to_value
        self.html_name = html_name
        self.output_dir = output_dir or "html"

        # Get KPI plots
        self.kpi_plots = KpiDataModel(input_data, output_data, html_name)

        # Generate plots and create HTML content
        self.plots_hash = self.generate_plots()
        
        # Initialize HtmlGenerator with plots_hash and parent_tabs
        HtmlGenerator(self.plots_hash, self.kpi_plots, self.html_name, self.output_dir)

    def is_in_regex_storage(self, signal_name):
        """
        Check if a signal name exists in the regex storage patterns.
        
        Args:
            signal_name: Name of the signal to check
            
        Returns:
            bool: True if signal exists in regex patterns, False otherwise
        """
        # Check if the signal name exists in any of the pattern lists
        return any(signal_name in patterns for patterns in SIGNAL_PATTERNS.values())

    def generate_plots(self):
        """
        Generates HTML content for plots by handling data and delegating scatter plot creation to PlotlyCharts.
        
        This method only handles the iteration through input and output data and collects data records.
        The actual plot generation and organization is handled by the PlotlyCharts class.

        Returns:
        - plots_hash: A dictionary containing plots organized by parent tabs.
        """
        plots_hash = {}
        
        # Get all unique keys from both data containers
        unique_keys = list(self.input_data.keys()) + list(self.output_data.keys())


        # Initialize a dictionary to hold figures/plots
        plots = {}
        
        inp_values = list(self.input_data.values())[0]
        out_values = list(self.output_data.values())[0]

        # Get the common range of groups that exist in both input and output
        common_groups = set(range(len(inp_values))) & set(range(len(out_values)))
        
        if not common_groups:
            print(f"Warning: No common groups found between input and output data in {self.html_name}")
            return plots_hash

        # Loop over common groups
        for grp_idx in common_groups:
            # Get the common range of plot indices that exist in both input and output for this group
            common_plots = set(range(len(inp_values[grp_idx]))) & set(range(len(out_values[grp_idx])))
            
            if not common_plots:
                print(f"Warning: No common plots found in group {grp_idx} between input and output data")
                continue

            # Loop over common plot indices
            for plt_idx in common_plots:
                # This loop creates one particular Plotly image
                key = f"{grp_idx}_{plt_idx}"
                fig_id = f"fig{key}"  # Unique identifier for the figure
                
                # List to hold data records for building the DataFrame for the scatter plot
                data_records = []
                
                signal_name = self.val_sig_map_in.get(f"{key}", "")

                # For each key (scan) in unique_keys, collect corresponding data
                for scan_idx, input_data, output_data in zip(unique_keys, self.input_data.values(), self.output_data.values()):
                    # Initialize data arrays
                    data_in_values = np.array([])
                    data_out_values = np.array([])

                    # this just for precaution for not going more then given data and it units is proper.and it will add the value.
                    if grp_idx < len(input_data) and plt_idx < len(input_data[grp_idx]):
                        data_in_values = input_data[grp_idx][plt_idx]
                    
                    if grp_idx < len(output_data) and plt_idx < len(output_data[grp_idx]):
                        data_out_values = output_data[grp_idx][plt_idx]
                    
                    if isinstance(data_in_values, (np.uint32, np.uint8, np.float32)):
                        data_in_values = [data_in_values]
                    if isinstance(data_out_values, (np.uint32, np.uint8, np.float32)):
                        data_out_values = [data_out_values]
                    if not isinstance(data_in_values, (list, np.ndarray)):
                        data_in_values = []
                    if not isinstance(data_out_values, (list, np.ndarray)):
                        data_out_values = []

                    # Find intersection and unique values
                    intersection = set(data_in_values).intersection(set(data_out_values))
                    unique_data_values = set(data_in_values) - intersection
                    unique_data_out_values = set(data_out_values) - intersection
                    
                    # Get the signal name for the current key
                    signal_name = self.val_sig_map_in.get(f"{key}", "")
                    
                    # Only add data records if the signal exists in regex storage
                    if self.is_in_regex_storage(signal_name):
                        # Add data records for intersection and unique values
                        if intersection:
                            data_records.extend([(scan_idx, val, f'Intersection value in {signal_name}') for val in intersection])
                        
                        if unique_data_values:
                            data_records.extend([(scan_idx, val, f'Input of {signal_name}') for val in unique_data_values])
                        
                        if unique_data_out_values:
                            data_records.extend([(scan_idx, val, f'Output of {signal_name}') for val in unique_data_out_values])
                
                # Only create plot if we have data records
                if data_records:
                    # Delegate plot creation to PlotlyCharts
                    fig = PlotlyCharts.create_data_comparison_scatter(data_records, key, self.val_sig_map_in)
                    plots[fig_id] = fig
                    
                    # Organize plots by parent tabs using PlotlyCharts
                    plots_hash = PlotlyCharts.organize_plots_by_tabs(plots, grp_idx, key, self.val_sig_map_in, plots_hash)
            
        return plots_hash
