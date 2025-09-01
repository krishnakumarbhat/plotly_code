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
        
    def set_stream_name(self, stream_name):
        """Set the stream name for plot organization"""
        self.stream_name = stream_name
    
    def scatter_with_scanindex(self, signal_name, data_records_in,data_records_out):
        """
        Create a scatter plot with scan index
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Data records to plot
        
        Returns:
        - fig_id: ID of the figure
        - fig: Plotly figure object
        """
        fig_id = f"scatter_fig_{signal_name}"
        fig = PlotlyCharts.scatter_with_scanindex(data_records_in,data_records_out, signal_name)
        return fig_id, fig

    def box_with_value(self, signal_name, data_records_in,data_records_out):
        """
        Create a box plot with values
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Data records to plot
        
        Returns:
        - box_fig_id: ID of the figure
        - box_fig: Plotly figure object
        """
        box_fig_id = f"box_fig_{signal_name}"
        box_fig = PlotlyCharts.create_data_comparison_box(data_records_in,data_records_out, signal_name)
        return box_fig_id, box_fig
    
    def cal_num_af_det(self,  signal_name, data_records_in,data_records_out):
        """
        Calculate number of detections
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Data records to analyze
        
        Returns:
        - fig_id: ID of the figure
        - fig: Plotly figure object (or None if not implemented)
        """
        fig_id = f"diff_in_{signal_name}"
        # Placeholder for future implementation
        fig = None
        return fig_id, fig
    
    def cal_num_af_det_with_bfstat_hist(self,  signal_name, data_records_in,data_records_out):
        """
        Calculate number of detections with histogram
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Data records to analyze
        
        Returns:
        - data_values: Processed data values
        """
        # Placeholder for future implementation
        return data_records_in,data_records_out
    
    def historgram_with_count(self,  signal_name, data_records_in,data_records_out):
        """
        Create a histogram with count
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Data records to plot
        
        Returns:
        - hist_fig_id: ID of the figure
        - hist_fig: Plotly figure object
        """
        hist_fig_id = f"hist_fig_{signal_name}"
        hist_fig = PlotlyCharts.create_data_comparison_histogram(data_records_in,data_records_out, signal_name)
        return hist_fig_id, hist_fig