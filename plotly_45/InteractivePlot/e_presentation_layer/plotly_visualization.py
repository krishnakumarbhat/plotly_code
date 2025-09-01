import plotly.graph_objects as go
import pandas as pd
import numpy as np

class PlotlyCharts:
    @staticmethod
    def scatter_with_scanindex(data_records, signal_name):
        """
        Create a scatter plot from data records with matched highlighting.
        
        Parameters:
        - data_records: Dictionary with 'I', 'O', 'M' keys each containing lists of [scan_idx, value] pairs
          - 'I':  mismatched Input values (not matching output)
          - 'O':  mismatched Output values (not matching input)
          - 'M': Matching values (both in input and output)
        - signal_name: Name of the signal for plot title and labels
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()

        # Define category names and colors
        categories = {
            'I': {'name': 'mismatched Values', 'color': 'red'},
            'O': {'name': 'mismatched Values', 'color': 'red'},
            'M': {'name': 'Matched Values', 'color': 'green'}
        }

        # Combine 'I' and 'O' data into a single trace
        combined_data = data_records['I'] + data_records['O']
        scan_indices = [item[0] for item in combined_data]
        values = [item[1] for item in combined_data]

        # Add trace for the combined 'I' and 'O' category
        fig.add_trace(go.Scattergl(
            x=scan_indices,
            y=values,
            mode='markers',
            name=categories['I']['name'],
            marker=dict(
                color=categories['I']['color'],
                size=12
            )
        ))

        # Add trace for the 'M' category
        if data_records['M']:
            scan_indices = [item[0] for item in data_records['M']]
            values = [item[1] for item in data_records['M']]
            fig.add_trace(go.Scattergl(
                x=scan_indices,
                y=values,
                mode='markers',
                name=categories['M']['name'],
                marker=dict(
                    color=categories['M']['color'],
                    size=12
                )
            ))

        # Update layout
        fig.update_layout(
            title=f'Scatter Plot of {signal_name}',
            xaxis_title='Scan Index',
            yaxis_title=f'Values of {signal_name}',
            legend_title='Legend',
            hovermode='closest'
        )
        
        return fig
    
    @staticmethod
    def create_match_mismatch_pie(data_records, signal_name):
        """
        Create a pie chart showing match and mismatch percentages.
        
        Parameters:
        - data_records: Dictionary containing match and mismatch counts
        - signal_name: Name of the signal for plot title and labels
        
        Returns:
        - fig: Plotly figure object
        """
        # Get match and mismatch counts
        match_count = data_records.get('match_count', 0)
        mismatch_count = data_records.get('mismatch_count', 0)
        total = match_count + mismatch_count
        
        # Calculate percentages
        if total > 0:
            match_percent = round((match_count / total) * 100, 2)
            mismatch_percent = round((mismatch_count / total) * 100, 2)
        else:
            match_percent = 0.0
            mismatch_percent = 0.0
        
        # Create pie chart data
        labels = ['Match', 'Mismatch']
        values = [match_percent, mismatch_percent]
        
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
            title=f'Match vs Mismatch for {signal_name}',
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