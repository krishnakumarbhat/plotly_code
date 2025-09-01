import numpy as np
import pandas as pd
from InteractivePlot.e_presentation_layer.plotly_visualization import PlotlyCharts 
from InteractivePlot.d_business_layer.data_retriver import DataRetriever
from functools import lru_cache
import plotly.graph_objects as go

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
    
    @lru_cache(maxsize=None)
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
    
    def create_match_mismatch_pie(self, signal_name, data_records,data_dict):
        """
        Create a pie chart showing match/mismatch percentages
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Dictionary with match and mismatch counts
        
        Returns:
        - fig_id: ID of the figure
        - fig: Plotly figure object
        """
        # if data_records['MI'] == []:
        #     pass
        fig_id = f"match_mismatch_pie_fig_{signal_name}"
        fig = PlotlyCharts.create_match_mismatch_pie(data_records, signal_name)
        return fig_id, fig
    
    def scatter_with_in_out(self, signal_name, data_records,data_dict):
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
        fig = PlotlyCharts.scatter_with_in_out(data_records, signal_name)
        return fig_id, fig
    
    def scatter_mismatch_scanindex(self, signal_name, data_records,data_dict):
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
        fig = PlotlyCharts.scatter_mismatch_scanindex(data_records, signal_name)
        return fig_id, fig

    def box_with_value(self, signal_name, data_records,data_dict):
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
    
    def cal_num_af_det(self, signal_name, data_records,data_dict):
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
    
    def cal_num_af_det_with_bfstat_hist(self, signal_name, data_records,data_dict):
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
    
    def create_diff_box_plot(self, signal_name, data_records,data_dict):
        """
        Create a box plot showing differences between input and output values.
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Dictionary with 'I', 'O', 'M' keys each containing lists of [scan_idx, value] pairs
        
        Returns:
        - fig_id: ID of the figure
        - fig: Plotly figure object
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
        
        # Create a map of scan indices to values for input and output
        input_map = {}
        output_map = {}
        
        for _, row in df.iterrows():
            if row['Type'] == 'Input':
                input_map[row['ScanIndex']] = row['Values']
            elif row['Type'] == 'Output':
                output_map[row['ScanIndex']] = row['Values']
        
        # Create a new dataframe for the differences
        diff_data = []
        
        # Find scan indices that have both input and output values
        common_indices = {k for k in input_map if k in output_map}
        
        for scan_idx in common_indices:
            diff_val = output_map[scan_idx] - input_map[scan_idx]
            diff_data.append({
                'ScanIndex': scan_idx,
                'Diff': diff_val
            })
        
        # Create DataFrame for differences
        if diff_data:
            diff_df = pd.DataFrame(diff_data)
            
            # Add the Diff column to the original DataFrame for filtering
            merged_df = df.copy()
            merged_df['Diff'] = merged_df['ScanIndex'].map(
                {row['ScanIndex']: row['Diff'] for row in diff_data}
            )
            
            diff_box_fig_id = f"diff_box_fig_{signal_name}"
            diff_box_fig = PlotlyCharts.create_diff_box_plot(diff_df, signal_name)
            return diff_box_fig_id, diff_box_fig
        else:
            # No matching pairs found
            diff_box_fig_id = f"diff_box_fig_{signal_name}"
            # Create an empty figure
            empty_fig = go.Figure()
            empty_fig.update_layout(
                title=f'No matching pairs found for {signal_name}',
                annotations=[{
                    'text': 'No data available for difference calculation',
                    'showarrow': False,
                    'font': {'size': 16}
                }]
            )
            return diff_box_fig_id, empty_fig
    
    def create_diff_scatter_plot(self, signal_name, data_records,data_dict):
        """
        Create a scatter plot showing differences between input and output values.
        
        Parameters:
        - signal_name: Name of the signal
        - data_records: Dictionary with 'I', 'O', 'M' keys each containing lists of [scan_idx, value] pairs
        
        Returns:
        - fig_id: ID of the figure
        - fig: Plotly figure object
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
        
        # Create a map of scan indices to values for input and output
        input_map = {}
        output_map = {}
        
        for _, row in df.iterrows():
            if row['Type'] == 'Input':
                input_map[row['ScanIndex']] = row['Values']
            elif row['Type'] == 'Output':
                output_map[row['ScanIndex']] = row['Values']
        
        # Create a new dataframe for the differences
        diff_data = []
        
        # Find scan indices that have both input and output values
        common_indices = {k for k in input_map if k in output_map}
        for scan_idx in common_indices:
            diff_val = output_map[scan_idx] - input_map[scan_idx]
            diff_data.append({
                'ScanIndex': scan_idx,
                'Diff': diff_val
            })
        
        # Create DataFrame for differences
        if diff_data:
            diff_df = pd.DataFrame(diff_data)
            
            diff_scatter_fig_id = f"diff_scatter_fig_{signal_name}"
            diff_scatter_fig = PlotlyCharts.create_diff_scatter_plot(diff_df, signal_name)
            return diff_scatter_fig_id, diff_scatter_fig
        else:
            # No matching pairs found
            diff_scatter_fig_id = f"diff_scatter_fig_{signal_name}"
            # Create an empty figure
            empty_fig = go.Figure()
            empty_fig.update_layout(
                title=f'No matching pairs found for {signal_name}',
                annotations=[{
                    'text': 'No data available for difference calculation',
                    'showarrow': False,
                    'font': {'size': 16}
                }]
            )
            return diff_scatter_fig_id, empty_fig
    
    def histogram_with_count(self, signal_name, data_records,data_dict):
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
        hist_fig = PlotlyCharts.histogram_with_count(df, signal_name)
        return hist_fig_id, hist_fig