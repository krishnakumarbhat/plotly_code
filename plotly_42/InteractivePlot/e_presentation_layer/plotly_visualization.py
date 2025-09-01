import plotly.graph_objects as go
import pandas as pd
import numpy as np

class PlotlyCharts:
    @staticmethod
    def scatter_with_scanindex(data_records_in, data_records_out, signal_name):
        """
        Create a scatter plot from data records with matched highlighting.
        
        Parameters:
        - data_records_in: Dictionary with scan_idx as key and [array_values, type] as value for input data
        - data_records_out: Dictionary with scan_idx as key and [array_values, type] as value for output data
        - signal_name: Name of the signal for plot title and labels
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()
        
        # Define color mapping
        color_map = {'matched': 'purple', 'input': 'blue', 'output': 'red'}
        
        # Process input data
        for scan_idx, (values, _) in data_records_in.items():
            # Check if this scan_idx also exists in output data
            is_matched = False
            if scan_idx in data_records_out:
                output_values, _ = data_records_out[scan_idx]
                # Compare input and output values
                if isinstance(values, np.ndarray) and isinstance(output_values, np.ndarray):
                    if np.array_equal(values, output_values):
                        is_matched = True
                elif values == output_values:
                    is_matched = True
            
            # Determine data type and color
            data_type = 'matched' if is_matched else 'input'
            color = color_map[data_type]
            
            # Handle array values
            if isinstance(values, np.ndarray):
                x_values = [scan_idx] * len(values)
                y_values = values.tolist() if hasattr(values, 'tolist') else values
                
                fig.add_trace(go.Scattergl(
                    x=x_values,
                    y=y_values,
                    mode='markers',
                    name=data_type,
                    marker=dict(
                        color=color,
                        opacity=0.7
                    )
                ))
            else:
                # Handle single values
                fig.add_trace(go.Scattergl(
                    x=[scan_idx],
                    y=[values],
                    mode='markers',
                    name=data_type,
                    marker=dict(
                        color=color,
                        opacity=0.7
                    )
                ))
        
        # Process output data that doesn't match with input
        for scan_idx, (values, _) in data_records_out.items():
            # Skip if already processed as matched
            if scan_idx in data_records_in:
                input_values, _ = data_records_in[scan_idx]
                if isinstance(values, np.ndarray) and isinstance(input_values, np.ndarray):
                    if np.array_equal(values, input_values):
                        continue
                elif values == input_values:
                    continue
            
            # Handle array values
            if isinstance(values, np.ndarray):
                x_values = [scan_idx] * len(values)
                y_values = values.tolist() if hasattr(values, 'tolist') else values
                
                fig.add_trace(go.Scattergl(
                    x=x_values,
                    y=y_values,
                    mode='markers',
                    name='output',
                    marker=dict(
                        color=color_map['output'],
                        opacity=0.7
                    )
                ))
            else:
                # Handle single values
                fig.add_trace(go.Scattergl(
                    x=[scan_idx],
                    y=[values],
                    mode='markers',
                    name='output',
                    marker=dict(
                        color=color_map['output'],
                        opacity=0.7
                    )
                ))
        
        fig.update_layout(
            title=f'Scatter Plot of {signal_name}',
            xaxis_title='ScanIndex',
            yaxis_title=f'Values of {signal_name}',
            legend_title='Legend',
            showlegend=True
        )
        
        return fig

    @staticmethod
    def create_data_comparison_box(data_records, signal_name):
        """
        Create a box plot from data records.
        
        Parameters:
        - data_records: List of tuples (scan_idx, val, type) for the plot
        - key: Key identifier for the plot title and labels
        - val_sig_map_in: Dictionary mapping signal identifiers
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()
        
        # Create a DataFrame from the data records
        df = pd.DataFrame(data_records, columns=['ScanIndex', 'Values', 'Type'])
        
        # Define color mapping
        color_map = {'Input': 'blue', 'Output': 'red'}
        
        # Iterate through each type and add a box trace
        for trace_type, color in color_map.items():
            trace_data = df[df['Type'].str.contains(trace_type)]
            if not trace_data.empty:
                fig.add_trace(go.Box(
                    y=trace_data['Values'],
                    name=trace_type,
                    marker_color=color
                ))
        
        fig.update_layout(
            title=f'Box Plot of {signal_name}',
            yaxis_title=f'Values of {signal_name}',
            legend_title='Legend'
        )
        
        return fig
        
    @staticmethod
    def create_data_comparison_histogram(data_records,signal_name):
        """
        Create a histogram from data records.
        
        Parameters:
        - data_records: List of tuples (scan_idx, val, type) for the plot
        - key: Key identifier for the plot title and labels
        - val_sig_map_in: Dictionary mapping signal identifiers
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()
        
        # Create a DataFrame from the data records
        df = pd.DataFrame(data_records, columns=['ScanIndex', 'Values', 'Type'])
        
        # Define color mapping
        color_map = {'Input': 'blue', 'Output': 'red'}
        
        # Iterate through each type and add a histogram trace
        for trace_type, color in color_map.items():
            trace_data = df[df['Type'].str.contains(trace_type)]
            if not trace_data.empty:
                fig.add_trace(go.Histogram(
                    x=trace_data['Values'],
                    name=trace_type,
                    marker_color=color,
                    opacity=0.7
                ))
        
        fig.update_layout(
            title=f'Histogram of {signal_name}',
            xaxis_title=f'Values of {signal_name}',
            yaxis_title='Count',
            legend_title='Legend',
            barmode='overlay'
        )
        
        return fig