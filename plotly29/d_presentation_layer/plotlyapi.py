import plotly.graph_objects as go

class PlotlyCharts:
    
    @staticmethod
    def create_scatter(x, y, title='Scatter Plot', x_label='X-axis', y_label='Y-axis'):
        """
        Create a basic scatter plot.
        
        Parameters:
        - x: List of x values
        - y: List of y values
        - title: Title of the plot
        - x_label: Label for the x-axis
        - y_label: Label for the y-axis
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure(data=go.Scatter(x=x, y=y, mode='markers'))
        fig.update_layout(title=title, xaxis_title=x_label, yaxis_title=y_label)
        return fig

    @staticmethod
    def create_intersection_scatter(data, data_out, title='Scatter Plot with Intersections', x_label='Keys', y_label='Values'):
        """
        Create a scatter plot with intersection handling.
        
        Parameters:
        - data: Dictionary with keys and lists of values for dataset 1
        - data_out: Dictionary with keys and lists of values for dataset 2
        - title: Title of the plot
        - x_label: Label for the x-axis
        - y_label: Label for the y-axis
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure()
        
        # Create a set to store unique keys
        unique_keys = set(data.keys()).union(set(data_out.keys()))

        # Iterate through each key to plot
        for key in unique_keys:
            # Get values from both datasets
            data_values = data.get(key, [])
            data_out_values = data_out.get(key, [])
            
            intersection = set(data_values).intersection(set(data_out_values))
            
            if intersection:
                fig.add_trace(go.Scatter(
                    x=[key] * len(intersection),
                    y=list(intersection),
                    mode='markers',
                    name=f'Intersection {key}',
                    marker=dict(color='purple', size=10)
                ))
            
            # Plot unique data points not in the intersection
            unique_data_values = set(data_values) - intersection
            if unique_data_values:
                fig.add_trace(go.Scatter(
                    x=[key] * len(unique_data_values),
                    y=list(unique_data_values),
                    mode='markers',
                    name=f'Data {key}',
                    marker=dict(color='blue')
                ))

            unique_data_out_values = set(data_out_values) - intersection
            if unique_data_out_values:
                fig.add_trace(go.Scatter(
                    x=[key] * len(unique_data_out_values),
                    y=list(unique_data_out_values),
                    mode='markers',
                    name=f'Data Out {key}',
                    marker=dict(color='red')
                ))

        # Update layout
        fig.update_layout(
            title=title,
            xaxis_title=x_label,
            yaxis_title=y_label,
            legend_title='Legend'
        )

        return fig

    @staticmethod
    def create_box(data, title='Box Plot', y_label='Values'):
        """
        Create a box plot.
        
        Parameters:
        - data: List of values to plot
        - title: Title of the plot
        - y_label: Label for the y-axis
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure(data=go.Box(y=data))
        fig.update_layout(title=title, yaxis_title=y_label)
        return fig

    @staticmethod
    def create_pie(labels, values, title='Pie Chart'):
        """
        Create a pie chart.
        
        Parameters:
        - labels: List of labels for each sector
        - values: List of values corresponding to each label
        - title: Title of the plot
        
        Returns:
        - fig: Plotly figure object
        """
        fig = go.Figure(data=[go.Pie(labels=labels, values=values)])
        fig.update_layout(title=title)
        return fig

