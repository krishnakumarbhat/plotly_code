import plotly.graph_objects as go
import pandas as pd
import numpy as np
from plotly.subplots import make_subplots

class PlotlyCharts:
    @staticmethod
    def scatter_with_scanindex(data_records, signal_name, sensor_position=""):
        """Create scatter plot with 4 distinct categories."""
        fig = go.Figure()
        
        # Category configuration
        categories = {
            'input': {'data': data_records['I'], 'name': 'Input', 'color': 'blue', 'symbol': 'circle'},
            'output': {'data': data_records['O'], 'name': 'Output', 'color': 'yellow', 'symbol': 'square'},
            'matched': {'data': data_records['M'], 'name': 'Matched', 'color': 'green', 'symbol': 'triangle-up'},
            'mismatched': {'data': data_records['I']+data_records['O'], 'name': 'Mismatched', 'color': 'red', 'symbol': 'x'}
        }

        # Add traces using Scattergl
        for key in ['input', 'output', 'matched', 'mismatched']:
            if categories[key]['data']:
                fig.add_trace(go.Scattergl(
                    x=[item[0] for item in categories[key]['data']],
                    y=[item[1] for item in categories[key]['data']],
                    mode='markers',
                    name=categories[key]['name'],
                    marker=dict(
                        color=categories[key]['color'],
                        symbol=categories[key]['symbol'],
                        size=3 if key == 'mismatched' else 2,
                        opacity=0.7
                    )
                ))

        # Layout configuration
        sensor_text = f" [{sensor_position}]" if sensor_position else ""
        fig.update_layout(
            title=f'Scatter Plot of {signal_name}{sensor_text}',
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
            xaxis_title=f'Values of {signal_name}',
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
            diff_data = df.drop_duplicates(subset=['ScanIndex'])
            
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
