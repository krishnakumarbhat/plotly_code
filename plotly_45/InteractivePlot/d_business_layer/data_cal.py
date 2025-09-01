import numpy as np
import pandas as pd
from InteractivePlot.e_presentation_layer.plotly_visualization import PlotlyCharts 
from InteractivePlot.d_business_layer.data_retriver import DataRetriever

class DataCalculations:
    """
    Class containing methods for processing and calculating derived data values
    for various radar signal parameters.
    """
    
    def __init__(self):
        """Initialize the DataCalculations class"""
        self.stream_name = None
        
    def create_match_mismatch_pie(self, signal_name, data_records):
        fig_id = f"match_mismatch_pie_fig_{signal_name}"
        fig = PlotlyCharts.scatter_with_scanindex(data_records, signal_name)
        return fig_id, fig

    def set_stream_name(self, stream_name):
        """Set the stream name for plot organization"""
        self.stream_name = stream_name
    
    def scatter_with_scanindex(self, signal_name, data_records):
        """
        Create a scatter plot with scan index
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Dictionary with 'I', 'O', 'M' keys each containing lists of [scan_idx, value] pairs
        
        Returns:
        - fig_id: ID of the figure
        - fig: Plotly figure object
        """
        fig_id = f"scatter_fig_{signal_name}"
        fig = PlotlyCharts.scatter_with_scanindex(data_records, signal_name)
        return fig_id, fig

    def box_with_value(self, signal_name, data_records):
        """
        Create a box plot with values
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Dictionary with 'I', 'O', 'M' keys each containing lists of [scan_idx, value] pairs
        
        Returns:
        - box_fig_id: ID of the figure
        - box_fig: Plotly figure object
        """
        # Convert the data_records structure to a format suitable for box plots
        box_data = []
        
        # Process input-only data points
        for item in data_records['I']:
            box_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Input'
            })
        
        # Process output-only data points
        for item in data_records['O']:
            box_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Output'
            })
        
        # Process matching data points (optional: can be included twice or with a different type)
        for item in data_records['M']:
            box_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Input'  # Can use 'Matching' or other label if the box plot is modified to show it
            })
            box_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Output'  # Adding the same point to Output for proper comparison
            })
        
        box_fig_id = f"box_fig_{signal_name}"
        box_fig = PlotlyCharts.create_data_comparison_box(box_data, signal_name)
        return box_fig_id, box_fig
    
    def cal_num_af_det(self, signal_name, data_records):
        """
        Calculate number of detections
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Dictionary with 'I', 'O', 'M' keys each containing lists of [scan_idx, value] pairs
        
        Returns:
        - fig_id: ID of the figure
        - fig: Plotly figure object (or None if not implemented)
        """
        fig_id = f"diff_in_{signal_name}"
        # Placeholder for future implementation
        fig = None
        return fig_id, fig
    
    def cal_num_af_det_with_bfstat_hist(self, signal_name, data_records):
        """
        Calculate number of detections with histogram
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Dictionary with 'I', 'O', 'M' keys each containing lists of [scan_idx, value] pairs
        
        Returns:
        - data_values: Processed data values
        """
        # Placeholder for future implementation
        return data_records
    
    def historgram_with_count(self, signal_name, data_records):
        """
        Create a histogram with count
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Dictionary with 'I', 'O', 'M' keys each containing lists of [scan_idx, value] pairs
        
        Returns:
        - hist_fig_id: ID of the figure
        - hist_fig: Plotly figure object
        """
        # Convert the data_records structure to a format suitable for histograms
        hist_data = []
        
        # Process input-only data points
        for item in data_records['I']:
            hist_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Input'
            })
        
        # Process output-only data points
        for item in data_records['O']:
            hist_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Output'
            })
        
        # Process matching data points (optional: can be included twice or with a different type)
        for item in data_records['M']:
            hist_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Input'  # Can use 'Matching' or other label if the histogram is modified to show it
            })
            hist_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Output'  # Adding the same point to Output for proper comparison
            })
            
        hist_fig_id = f"hist_fig_{signal_name}"
        hist_fig = PlotlyCharts.create_data_comparison_histogram(hist_data, signal_name)
        return hist_fig_id, hist_fig