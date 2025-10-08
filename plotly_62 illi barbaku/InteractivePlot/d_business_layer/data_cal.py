import numpy as np
import pandas as pd
from InteractivePlot.e_presentation_layer.plotly_visualization import PlotlyCharts 
from InteractivePlot.d_business_layer.data_retriver import DataRetriever
from functools import lru_cache

class DataCalculations:
    """
    Class containing methods for processing and calculating derived data values
    for various radar signal parameters.
    """
    
    def __init__(self):
        """Initialize the DataCalculations class"""
        self.stream_name = None
        self._data_frame_cache = {}
        self._current_data = None
        
    def set_stream_name(self, stream_name):
        """Set the stream name for plot organization"""
        self.stream_name = stream_name
    
    @lru_cache(maxsize=32)
    def _convert_to_dataframe(self, signal_name, data_records_hash):
        """
        Convert the data records to a DataFrame for efficient processing.
        This cached method prevents redundant conversions for the same data.
        
        Parameters:
        - signal_name: Name of the signal (used for cache key)
        - data_records_hash: Hash of the data_records dictionary
        
        Returns:
        - DataFrame with columns: ScanIndex, Values, Type
        """
        # data_records_hash is just used for the cache key
        # Actual data is stored in the instance variable
        data_records = self._current_data
        
        # Create a list to hold all data points
        all_data = []
        
        # Process input-only data points
        for item in data_records['I']:
            all_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Input'
            })
        
        # Process output-only data points
        for item in data_records['O']:
            all_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Output'
            })
        
        # Process matching data points (include as both Input and Output for proper comparison)
        for item in data_records['M']:
            all_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Input'
            })
            all_data.append({
                'ScanIndex': item[0],
                'Values': item[1],
                'Type': 'Output'
            })
        
        # Convert to DataFrame
        return pd.DataFrame(all_data)
    
    def create_match_mismatch_pie(self, signal_name, data_records):
        """
        Create a pie chart showing match/mismatch percentages
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Dictionary with match and mismatch counts
        
        Returns:
        - fig_id: ID of the figure
        - fig: Plotly figure object
        """
        fig_id = f"match_mismatch_pie_fig_{signal_name}"
        fig = PlotlyCharts.create_match_mismatch_pie(data_records, signal_name)
        return fig_id, fig
    
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
        # Store current data for the cached function to use
        self._current_data = data_records
        
        # Create a hash of the data for cache key
        data_hash = hash(
            tuple(map(tuple, data_records['I'])) + 
            tuple(map(tuple, data_records['O'])) + 
            tuple(map(tuple, data_records['M']))
        )
        
        # Get or convert to DataFrame
        df = self._convert_to_dataframe(signal_name, data_hash)
        
        box_fig_id = f"box_fig_{signal_name}"
        box_fig = PlotlyCharts.create_data_comparison_box(df, signal_name)
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
        # Store current data for the cached function to use
        self._current_data = data_records
        
        # Create a hash of the data for cache key
        data_hash = hash(
            tuple(map(tuple, data_records['I'])) + 
            tuple(map(tuple, data_records['O'])) + 
            tuple(map(tuple, data_records['M']))
        )
        
        # Get or convert to DataFrame
        df = self._convert_to_dataframe(signal_name, data_hash)
            
        hist_fig_id = f"hist_fig_{signal_name}"
        hist_fig = PlotlyCharts.create_data_comparison_histogram(df, signal_name)
        return hist_fig_id, hist_fig