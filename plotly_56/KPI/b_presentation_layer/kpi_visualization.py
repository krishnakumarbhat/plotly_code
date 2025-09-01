import plotly.graph_objects as go
from plotly.subplots import make_subplots

class KpiVisualization:
    @staticmethod
    def kpi_scatter_plot(data_records, signal_name, sensor_position=""):
        """
        Create a specialized scatter plot for KPI data showing detection matching.
        
        Parameters:
        - data_records: Dictionary with 'I', 'O', 'M' keys each containing lists of [scan_idx, value] pairs
            - 'I':  mismatched Input values (not matching output)
            - 'O':  mismatched Output values (not matching input)
            - 'M': Matching values (both in input and output)
        - signal_name: Name of the signal for plot title and labels
        - sensor_position: Position of the sensor (KPI)
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()

        # Define category names and colors for KPI visualization
        categories = {
            'I': {'name': 'Input Mismatches', 'color': 'red', 'opacity': 0.7},
            'O': {'name': 'Output Mismatches', 'color': 'orange', 'opacity': 0.7},
            'M': {'name': 'Matched Detections', 'color': 'green', 'opacity': 0.2}
        }

        # Add trace for the 'M' category (matched detections)
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
                    opacity=categories['M']['opacity'],
                    size=4
                ),
                hovertemplate='Scan Index: %{x}<br>Value: %{y:.2f}'
            ))

        # Combine 'I' and 'O' data into separate traces
        if data_records['I']:
            scan_indices = [item[0] for item in data_records['I']]
            values = [item[1] for item in data_records['I']]
            fig.add_trace(go.Scattergl(
                x=scan_indices,
                y=values,
                mode='markers',
                name=categories['I']['name'],
                marker=dict(
                    color=categories['I']['color'],
                    opacity=categories['I']['opacity'],
                    size=4
                ),
                hovertemplate='Scan Index: %{x}<br>Value: %{y:.2f}'
            ))

        if data_records['O']:
            scan_indices = [item[0] for item in data_records['O']]
            values = [item[1] for item in data_records['O']]
            fig.add_trace(go.Scattergl(
                x=scan_indices,
                y=values,
                mode='markers',
                name=categories['O']['name'],
                marker=dict(
                    color=categories['O']['color'],
                    opacity=categories['O']['opacity'],
                    size=4
                ),
                hovertemplate='Scan Index: %{x}<br>Value: %{y:.2f}'
            ))

        # Update layout with KPI-specific styling
        fig.update_layout(
            title=f'KPI Detection Matching - {signal_name}{sensor_position}',
            xaxis_title='Scan Index',
            yaxis_title=f'Detection Values',
            legend_title='Detection Type',
            hovermode='closest',
            template='plotly_white',
            margin=dict(l=50, r=50, t=80, b=50),
            height=600,
            width=1000
        )

        # Add annotations for key metrics
        total_points = sum(len(data_records[key]) for key in ['I', 'O', 'M'])
        match_percentage = (len(data_records['M']) / total_points * 100) if total_points > 0 else 0
        
        fig.add_annotation(
            text=f'Match Rate: {match_percentage:.1f}%<br>Total Detections: {total_points}',
            xref='paper', yref='paper',
            x=0.95, y=0.95,
            showarrow=False,
            bgcolor='rgba(255, 255, 255, 0.8)',
            bordercolor='black',
            borderwidth=1
        )

        return fig

    @staticmethod
    def kpi_pie_chart(data_records, signal_name, sensor_position=""):
        """
        Create a pie chart showing detection matching statistics.
        
        Parameters:
        - data_records: Dictionary containing match and mismatch counts
        - signal_name: Name of the signal for plot title and labels
        - sensor_position: Position of the sensor (KPI)
        
        Returns:
        - fig: Plotly figure object
        """
        # Get match and mismatch counts
        match_count = len(data_records.get('M', []))
        mismatch_count = len(data_records.get('I', [])) + len(data_records.get('O', []))
        total = match_count + mismatch_count
        
        # Calculate percentages
        match_percent = (match_count / total * 100) if total > 0 else 0
        mismatch_percent = (mismatch_count / total * 100) if total > 0 else 0
        
        # Create pie chart
        fig = go.Figure(data=[go.Pie(
            labels=['Matched Detections', 'Mismatched Detections'],
            values=[match_count, mismatch_count],
            marker=dict(colors=['green', 'red']),
            textinfo='label+percent',
            textposition='inside',
            hole=0.4,
            showlegend=True
        )])
        
        # Update layout
        fig.update_layout(
            title=f'Detection Matching Statistics - {signal_name}{sensor_position}',
            legend_title='Detection Type',
            template='plotly_white',
            height=400,
            width=600
        )
        
        return fig
