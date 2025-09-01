import plotly.graph_objects as go
import pandas as pd
import numpy as np
from plotly.subplots import make_subplots
import logging
import gc
import sys
import os
import time

# # Try to import datashader utilities
# try:
from InteractivePlot.e_presentation_layer.datashader_utils import (
    create_datashader_scatter, 
    create_datashader_categorized_scatter,
    # DATASHADER_AVAILABLE
    )
# except ImportError:
#     # If datashader is not available, we'll fall back to Plotly Scattergl
#     DATASHADER_AVAILABLE = False

class PlotlyCharts:
    @staticmethod
    def scatter_with_in_out(data_records, signal_name, sensor_position=""):
        a= time.time()
        """Create scatter plot with 4 distinct categories using datashader for memory efficiency."""
        # Category configuration
        categories = {
            'input': {'data': data_records['I'], 'name': 'Input', 'color': 'blue', 'symbol': 'circle'},
            'output': {'data': data_records['O'], 'name': 'Output', 'color': 'red', 'symbol': 'square'},
        }
        
        # Check data size
        total_points = sum(len(categories[key]['data']) for key in ['input', 'output'] if categories[key]['data'])
        use_datashader = False and (total_points > 1000)
        
        if use_datashader:
            # Use datashader for large datasets
            try:
                # Prepare data for datashader
                data_dict = {}
                for key in ['input', 'output']:
                    if categories[key]['data']:
                        data_dict[categories[key]['name']] = {
                            'x': [item[0] for item in categories[key]['data']],
                            'y': [item[1] for item in categories[key]['data']]
                        }
                        
                # Use datashader for memory-efficient visualization
                sensor_text = f" [{sensor_position}]" if sensor_position else ""
                fig = create_datashader_categorized_scatter(
                    data_dict,
                    width=1000,
                    height=600,
                    title=f'Scatter In/Out Plot of {signal_name}{sensor_text}',
                    x_label='Scan Index',
                    y_label=f'Values of {signal_name}'
                )
                
                # Force garbage collection to free memory
                data_dict = None
                gc.collect()

                return fig
                
            except Exception as e:
                logging.error(f"Datashader rendering failed: {str(e)}. Falling back to Scattergl.")
                # Fall back to Scattergl if datashader fails
        
        # If datashader not used or failed, use Scattergl
        fig = go.Figure()
        
        # Add traces using Scattergl
        for key in ['input', 'output']:
            if categories[key]['data']:
                fig.add_trace(go.Scattergl(
                    x=[item[0] for item in categories[key]['data']],
                    y=[item[1] for item in categories[key]['data']],
                    mode='markers',
                    name=categories[key]['name'],
                    marker=dict(
                        color=categories[key]['color'],
                        symbol=categories[key]['symbol'],
                        size=4,
                        opacity=0.7
                    )
                ))
                
                # Force memory cleanup after each trace
                gc.collect()

        # Layout configuration
        sensor_text = f" [{sensor_position}]" if sensor_position else ""
        fig.update_layout(
            title=f'Scatter In/Out Plot of {signal_name}{sensor_text}',
            xaxis_title='Scan Index',
            yaxis_title=f'Values of {signal_name}',
            legend_title='Categories',
            hovermode='closest'
        )
        b = time.time()
        print(f"time taken by scatter api as whole of {b-a} in {signal_name}")
        return fig   
        
    @staticmethod
    def scatter_mismatch_scanindex(data_records, signal_name, sensor_position=""):
        """Create scatter plot for mismatched points using datashader for memory efficiency."""
        # Skip if both MI and MO are empty
        if not data_records['MI'] and not data_records['MO']:
            return None
            
        # Category configuration
        categories = {
            # 'matched': {'data': data_records['M'], 'name': 'Matched', 'color': 'green', 'symbol': 'triangle-up'},
            'mismatched': {'data': data_records['MI']+data_records['MO'], 'name': 'Mismatched', 'color': 'red', 'symbol': 'x'}
        }
        
        # Check data size
        total_points = len(categories['mismatched']['data']) if categories['mismatched']['data'] else 0
        use_datashader = total_points > 10
        
        if use_datashader:
            # Use datashader for large datasets
            try:
                # Prepare data for datashader
                if categories['mismatched']['data']:
                    x_data = [item[0] for item in categories['mismatched']['data']]
                    y_data = [item[1] for item in categories['mismatched']['data']]
                    
                    # Use datashader for memory-efficient visualization
                    sensor_text = f" [{sensor_position}]" if sensor_position else ""
                    fig = create_datashader_scatter(
                        x_data, 
                        y_data,
                        width=1000,
                        height=600,
                        colormap='reds',  # Use a reddish colormap for mismatches
                        title=f'Scatter Mismatch Plot of {signal_name}{sensor_text}',
                        x_label='Scan Index',
                        y_label=f'Values of {signal_name}'
                    )
                    
                    # Force garbage collection to free memory
                    x_data = None
                    y_data = None
                    gc.collect()
                    
                    return fig
                    
            except Exception as e:
                logging.error(f"Datashader rendering failed for mismatch: {str(e)}. Falling back to Scattergl.")
                # Fall back to Scattergl if datashader fails
        
        # If datashader not used or failed, use Scattergl
        fig = go.Figure()
        
        # Add traces using Scattergl
        for key in ['mismatched']:
            if categories[key]['data']:
                fig.add_trace(go.Scattergl(
                    x=[item[0] for item in categories[key]['data']],
                    y=[item[1] for item in categories[key]['data']],
                    mode='markers',
                    name=categories[key]['name'],
                    marker=dict(
                        color=categories[key]['color'],
                        symbol=categories[key]['symbol'],
                        size=5,
                        opacity=0.7
                    )
                ))
                
                # Force memory cleanup
                gc.collect()
                
        # Layout configuration
        sensor_text = f" [{sensor_position}]" if sensor_position else ""
        fig.update_layout(
            title=f'Scatter Mismatch Plot of {signal_name}{sensor_text}',
            xaxis_title='Scan Index',
            yaxis_title=f'Values of {signal_name}',
            legend_title='Categories',
            hovermode='closest'
        )
        
        return fig
    
    @staticmethod
    def create_match_mismatch_pie(data_records, signal_name, sensor_position=""):
        """
        Create a pie chart showing match and mismatch percentages.
        
        Parameters:
        - data_records: Dictionary containing match and mismatch counts
        - signal_name: Name of the signal for plot title and labels
        - sensor_position: Position of the sensor (RL/RR/FL/FR)
        
        Returns:
        - fig: Plotly figure object
        """
        # Get match and mismatch counts
        match_count = data_records.get('match_count', 0)
        mismatch_count = data_records.get('mismatch_count', 0)
        total = match_count + mismatch_count
        
        # Calculate percentages
        if total > 0:
            match_percent = round((match_count / total) * 100, 10)
            mismatch_percent = round((mismatch_count / total) * 100, 10)
        else:
            match_percent = 0.0
            mismatch_percent = 0.0
        
        # Create pie chart data
        labels = ['Match', 'Mismatch']
        values = [match_percent, mismatch_percent]
        
        # Sensor position text for title
        sensor_text = f" [{sensor_position}]" if sensor_position else ""
        
        # Create pie chart
        fig = go.Figure(data=[go.Pie(
            labels=labels,
            values=values,
            textinfo='label+percent',
            insidetextorientation='radial',
            marker=dict(
                colors=['green', 'red'],
                line=dict(color='#000000', width=2)
            )
        )])
        
        # Update layout
        fig.update_layout(
            title=f'Match vs Mismatch for {signal_name}{sensor_text}',
            legend_title='Legend',
            showlegend=True
        )
        
        return fig
    @staticmethod
    def histogram_with_count(df, signal_name, sensor_position=""):
        """
        Create a histogram from DataFrame.
        
        Parameters:
        - df: DataFrame with columns: ScanIndex, Values, Type
        - signal_name: Signal name for the plot title and labels
        - sensor_position: Position of the sensor (RL/RR/FL/FR)
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()
        
        # Define color mapping
        color_map = {'Input': 'blue', 'Output': 'red'}
        
        # Convert radians to degrees for azimuth and elevation
        is_angular_measurement = signal_name.lower() in ['azimuth', 'elevation']
        if is_angular_measurement and not df.empty:
            import math
            # RAD2DEG(x) ((x * 180.00) / PI)
            df['Values'] = df['Values'].apply(lambda x: (x * 180.0) / math.pi)
            unit_suffix = " (degrees)"
        else:
            unit_suffix = ""
        
        # Use nbins for better performance with large datasets
        nbins = min(50, int(df['Values'].nunique() * 0.5) + 1) if not df.empty else 20
        
        # Iterate through each type and add a histogram trace
        for trace_type, color in color_map.items():
            trace_data = df[df['Type'] == trace_type]
            if not trace_data.empty:
                fig.add_trace(go.Histogram(
                    x=trace_data['Values'],
                    name=trace_type,
                    marker_color=color,
                    opacity=0.7,
                    nbinsx=nbins,
                    histnorm='probability density'  # Normalize to show probability density
                ))
        
        # Sensor position text for title
        sensor_text = f" [{sensor_position}]" if sensor_position else ""
        
        fig.update_layout(
            title=f'Histogram of {signal_name}{sensor_text}',
            xaxis_title=f'Values of {signal_name}{unit_suffix}',
            yaxis_title='Probability Density',
            legend_title='Legend',
            barmode='overlay'
        )
        
        return fig
    
    @staticmethod
    def create_data_comparison_box(df, signal_name, sensor_position=""):
        """
        Create a box plot from DataFrame.
        
        Parameters:
        - df: DataFrame with columns: ScanIndex, Values, Type
        - signal_name: Signal name for the plot title and labels
        - sensor_position: Position of the sensor (RL/RR/FL/FR)
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()
        
        # Define color mapping
        color_map = {'Input': 'blue', 'Output': 'red'}
        
        # Iterate through each type and add a box trace
        for trace_type, color in color_map.items():
            trace_data = df[df['Type'] == trace_type]
            if not trace_data.empty:
                fig.add_trace(go.Box(
                    y=trace_data['Values'],
                    name=trace_type,
                    marker_color=color
                ))
        
        # Sensor position text for title
        sensor_text = f" [{sensor_position}]" if sensor_position else ""
        
        fig.update_layout(
            title=f'Box Plot of {signal_name}{sensor_text}',
            yaxis_title=f'Values of {signal_name}',
            legend_title='Legend'
        )
        
        return fig

    @staticmethod
    def create_diff_box_plot(df, signal_name, sensor_position=""):
        """
        Create a box plot showing differences between input and output values.
        
        Parameters:
        - df: DataFrame with columns: ScanIndex, Values, Type, and Diff
        - signal_name: Signal name for the plot title and labels
        - sensor_position: Position of the sensor (RL/RR/FL/FR)
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()
        
        # Filter data to get scan indexes that have both input and output values
        if not df.empty and 'Diff' in df.columns:
            # Add box plot for difference values
            fig.add_trace(go.Box(
                y=df['Diff'],
                name='Difference',
                marker_color='purple'
            ))
            
            # Add zero line for reference
            fig.add_shape(
                type="line",
                x0=-0.5,
                x1=0.5,
                y0=0,
                y1=0,
                line=dict(
                    color="black",
                    width=2,
                    dash="dash",
                )
            )
        
        # Sensor position text for title
        sensor_text = f" [{sensor_position}]" if sensor_position else ""
        
        fig.update_layout(
            title=f'Difference Box Plot of {signal_name}{sensor_text}',
            yaxis_title=f'Difference (Output - Input)',
            legend_title='Legend',
            showlegend=False
        )
        
        return fig
    
    @staticmethod
    def create_diff_scatter_plot(df, signal_name, sensor_position=""):
        """
        Create a scatter plot showing differences between input and output values.
        Uses datashader for memory-efficient visualization without data reduction.
        
        Parameters:
        - df: DataFrame with columns: ScanIndex, Values, Type, and Diff
        - signal_name: Signal name for the plot title and labels
        - sensor_position: Position of the sensor (RL/RR/FL/FR)
        
        Returns:
        - fig: Plotly figure object
        """
        # Filter data to get scan indexes that have both input and output values
        if not df.empty and 'Diff' in df.columns:
            diff_data = df.drop_duplicates(subset=['ScanIndex'])
            
            # Check data size
            use_datashader = len(diff_data) > 10
            
            if use_datashader:
                # Use datashader for large datasets
                try:
                    # Get min/max for x-range to add zero line reference
                    x_min = diff_data['ScanIndex'].min()
                    x_max = diff_data['ScanIndex'].max()
                    
                    # Use datashader for memory-efficient visualization
                    sensor_text = f" [{sensor_position}]" if sensor_position else ""
                    fig = create_datashader_scatter(
                        diff_data['ScanIndex'], 
                        diff_data['Diff'],
                        width=1000,
                        height=600,
                        colormap='purples',  # Use a purple colormap for differences
                        title=f'Difference Scatter Plot of {signal_name}{sensor_text}',
                        x_label='Scan Index',
                        y_label=f'Difference (Output - Input)'
                    )
                    
                    # Add zero line for reference
                    fig.add_shape(
                        type="line",
                        x0=x_min,
                        x1=x_max,
                        y0=0,
                        y1=0,
                        line=dict(
                            color="black",
                            width=2,
                            dash="dash",
                        )
                    )
                    
                    # Release memory
                    gc.collect()
                    
                    return fig
                    
                except Exception as e:
                    logging.error(f"Datashader rendering failed for diff scatter: {str(e)}. Falling back to Scattergl.")
                    # Fall back to Scattergl if datashader fails
            
            # If datashader not used or failed, use Scattergl
            fig = go.Figure()
            
            # Add scatter plot for difference values
            fig.add_trace(go.Scattergl(
                x=diff_data['ScanIndex'],
                y=diff_data['Diff'],
                mode='markers',
                name='Difference',
                marker=dict(
                    color='purple',
                    size=6,
                    opacity=0.7
                )
            ))
            
            # Add zero line for reference
            fig.add_shape(
                type="line",
                x0=diff_data['ScanIndex'].min(),
                x1=diff_data['ScanIndex'].max(),
                y0=0,
                y1=0,
                line=dict(
                    color="black",
                    width=2,
                    dash="dash",
                )
            )
            
            # Sensor position text for title
            sensor_text = f" [{sensor_position}]" if sensor_position else ""
            
            fig.update_layout(
                title=f'Difference Scatter Plot of {signal_name}{sensor_text}',
                xaxis_title='Scan Index',
                yaxis_title=f'Difference (Output - Input)',
                hovermode='closest'
            )
            
            return fig
        
        # Empty figure if no data
        fig = go.Figure()
        sensor_text = f" [{sensor_position}]" if sensor_position else ""
        fig.update_layout(
            title=f'Difference Scatter Plot of {signal_name}{sensor_text}',
            xaxis_title='Scan Index',
            yaxis_title=f'Difference (Output - Input)',
            hovermode='closest',
            annotations=[{
                'text': 'No data available for difference calculation',
                'showarrow': False,
                'font': {'size': 16}
            }]
        )
        return fig
