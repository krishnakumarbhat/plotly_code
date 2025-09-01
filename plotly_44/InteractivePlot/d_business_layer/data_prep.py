import plotly.graph_objects as go
import pandas as pd
import numpy as np
import multiprocessing as mp
from functools import partial
import os
import copy
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
        self.output_dir = output_dir or "html"  # Default to "html" if not provided

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

    def _extract_plot_data(self, plot_info, all_data):
        """
        Extract all necessary data for a single plot task to be processed in parallel.
        This creates a self-contained data package that can be safely pickled.
        
        Args:
            plot_info: Tuple containing (grp_idx, plt_idx) for the plot
            all_data: Dictionary with all data needed for processing
            
        Returns:
            Dict: A dictionary with all data needed for this specific plot
        """
        grp_idx, plt_idx = plot_info
        
        # Create a task-specific data package that includes only what's needed for this plot
        plot_data = {
            'grp_idx': grp_idx,
            'plt_idx': plt_idx,
            'unique_keys': all_data['unique_keys'],
            'input_data': {},
            'output_data': {},
            'val_sig_map_in': all_data['val_sig_map_in']
        }
        
        # Extract only the relevant data for this plot task
        for scan_idx in all_data['unique_keys']:
            if scan_idx in all_data['input_data'] and grp_idx < len(all_data['input_data'][scan_idx]):
                if plt_idx < len(all_data['input_data'][scan_idx][grp_idx]):
                    # Only extract the specific data needed for this plot
                    if scan_idx not in plot_data['input_data']:
                        plot_data['input_data'][scan_idx] = {}
                    plot_data['input_data'][scan_idx] = all_data['input_data'][scan_idx][grp_idx][plt_idx]
                        
            if scan_idx in all_data['output_data'] and grp_idx < len(all_data['output_data'][scan_idx]):
                if plt_idx < len(all_data['output_data'][scan_idx][grp_idx]):
                    # Only extract the specific data needed for this plot
                    if scan_idx not in plot_data['output_data']:
                        plot_data['output_data'][scan_idx] = {}
                    plot_data['output_data'][scan_idx] = all_data['output_data'][scan_idx][grp_idx][plt_idx]
        
        return plot_data

    def _process_plot_data(self, plot_data):
        """
        Process data for a single plot - can be run in parallel.
        
        Args:
            plot_data: Dictionary containing all data for this plot
            
        Returns:
            Tuple containing (fig_id, fig, grp_idx, key, should_include) where:
                - fig_id: Unique identifier for the figure
                - fig: The plotly figure object
                - grp_idx: Group index
                - key: Key identifier for the plot
                - should_include: Boolean indicating if the plot should be included
        """
        grp_idx = plot_data['grp_idx']
        plt_idx = plot_data['plt_idx']
        key = f"{grp_idx}_{plt_idx}"
        fig_id = f"fig{key}"  # Unique identifier for the figure
        
        # List to hold data records for building the DataFrame for the scatter plot
        data_records = []
        
        # For each key (scan) in unique_keys, collect corresponding data
        for scan_idx in plot_data['unique_keys']:
            # Initialize data arrays
            data_in_values = np.array([])
            data_out_values = np.array([])

            # Get data from the pre-extracted task-specific data
            if scan_idx in plot_data['input_data']:
                data_in_values = plot_data['input_data'][scan_idx]
            
            if scan_idx in plot_data['output_data']:
                data_out_values = plot_data['output_data'][scan_idx]
            
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
            signal_name = plot_data['val_sig_map_in'].get(f"{key}", "")
            
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
            fig = PlotlyCharts.create_data_comparison_scatter(data_records, key, plot_data['val_sig_map_in'])
            return (fig_id, fig, grp_idx, key, True)
        else:
            return (fig_id, None, grp_idx, key, False)

    def generate_plots(self):
        """
        Generates HTML content for plots by handling data and delegating scatter plot creation to PlotlyCharts.
        Uses multiprocessing to generate plots in parallel.

        Returns:
        - plots_hash: A dictionary containing plots organized by parent tabs.
        """
        plots_hash = {}
        
        # Get all unique keys from both data containers
        unique_keys = list(self.input_data.keys()) + list(self.output_data.keys())

        # Initialize a dictionary to hold figures/plots
        plots = {}
        
        # Check if we have input and output data
        if not self.input_data or not self.output_data:
            print(f"Warning: Missing input or output data for {self.html_name}")
            return plots_hash
        
        inp_values = list(self.input_data.values())[0]
        out_values = list(self.output_data.values())[0]

        # Get the common range of groups that exist in both input and output
        common_groups = set(range(len(inp_values))) & set(range(len(out_values)))
        
        if not common_groups:
            print(f"Warning: No common groups found between input and output data in {self.html_name}")
            return plots_hash
        
        # Prepare all data that will be needed for plot generation
        all_data = {
            'unique_keys': unique_keys,
            'input_data': self.input_data,
            'output_data': self.output_data,
            'val_sig_map_in': self.val_sig_map_in
        }
        
        # Collect all plot tasks to process in parallel
        plot_tasks = []
        plot_data_packages = []
        
        for grp_idx in common_groups:
            # Get the common range of plot indices that exist in both input and output for this group
            common_plots = set(range(len(inp_values[grp_idx]))) & set(range(len(out_values[grp_idx])))
            
            if not common_plots:
                print(f"Warning: No common plots found in group {grp_idx} between input and output data")
                continue

            # Add plot tasks
            for plt_idx in common_plots:
                plot_info = (grp_idx, plt_idx)
                plot_tasks.append(plot_info)
                
                # Extract and package the data needed for this plot
                plot_data_package = self._extract_plot_data(plot_info, all_data)
                plot_data_packages.append(plot_data_package)
        
        # Determine the number of processes - use at most the number of CPU cores or tasks, whichever is smaller
        num_processes = min(os.cpu_count() or 4, len(plot_tasks))
        
        if plot_data_packages:
            print(f"Generating {len(plot_data_packages)} plots in parallel using {num_processes} processes...")
            
            # Create a process pool
            with mp.Pool(processes=num_processes) as pool:
                # Process all plot tasks in parallel with the pre-packaged data
                results = pool.map(self._process_plot_data, plot_data_packages)
                
                # Process the results
                for fig_id, fig, grp_idx, key, should_include in results:
                    if should_include and fig is not None:
                        plots[fig_id] = fig
                        # Organize plots by parent tabs
                        plots_hash = PlotlyCharts.organize_plots_by_tabs(plots, grp_idx, key, self.val_sig_map_in, plots_hash)
        
        print(f"Generated {len(plots)} plots for {self.html_name}")
        return plots_hash
