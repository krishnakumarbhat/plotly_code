import plotly.graph_objects as go

class PlotlyCharts:
    
    @staticmethod
    def create_scatter(x, y, title='Scatter Plot', x_label='X-axis', y_label='Y-axis'):
        """
        Create a scatter plot.
        
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

