from typing import List, Any, Optional, Dict
from dataclasses import dataclass
import plotly.graph_objects as go
import plotly.express as px
import pandas as pd
import numpy as np

from InteractivePlot.d_business_layer.data_prep import PlotConfig

@dataclass
class VisualizationConfig:
    """Configuration for visualization appearance and behavior"""
    template: str = "plotly_white"
    width: int = 800
    height: int = 600
    margin: Dict[str, int] = None
    
    def __post_init__(self):
        if self.margin is None:
            self.margin = {"l": 50, "r": 50, "t": 50, "b": 50}

class PlotlyCharts:
    """
    Factory class for creating various types of Plotly visualizations
    with consistent styling and configuration.
    """
    
    def __init__(self, config: Optional[VisualizationConfig] = None):
        """
        Initialize the chart factory with configuration.
        
        Args:
            config: Optional visualization configuration
        """
        self.config = config or VisualizationConfig()
    
    def create_scatter_plot(self, x_data: List[Any], y_data: List[Any], 
                          plot_config: PlotConfig) -> go.Figure:
        """
        Create a scatter plot with the given data and configuration.
        
        Args:
            x_data: Data for x-axis
            y_data: Data for y-axis
            plot_config: Configuration for the plot
            
        Returns:
            Configured plotly figure
        """
        fig = go.Figure()
        
        fig.add_trace(go.Scatter(
            x=x_data,
            y=y_data,
            mode='markers',
            name=plot_config.y_label,
            marker=dict(
                color=plot_config.color,
                opacity=plot_config.opacity
            )
        ))
        
        self._apply_layout(fig, plot_config)
        return fig
    
    def create_line_plot(self, x_data: List[Any], y_data: List[Any],
                        plot_config: PlotConfig) -> go.Figure:
        """
        Create a line plot with the given data and configuration.
        
        Args:
            x_data: Data for x-axis
            y_data: Data for y-axis
            plot_config: Configuration for the plot
            
        Returns:
            Configured plotly figure
        """
        fig = go.Figure()
        
        fig.add_trace(go.Scatter(
            x=x_data,
            y=y_data,
            mode='lines',
            name=plot_config.y_label,
            line=dict(
                color=plot_config.color,
                width=2
            )
        ))
        
        self._apply_layout(fig, plot_config)
        return fig
    
    def create_bar_plot(self, x_data: List[Any], y_data: List[Any],
                       plot_config: PlotConfig) -> go.Figure:
        """
        Create a bar plot with the given data and configuration.
        
        Args:
            x_data: Data for x-axis
            y_data: Data for y-axis
            plot_config: Configuration for the plot
            
        Returns:
            Configured plotly figure
        """
        fig = go.Figure()
        
        fig.add_trace(go.Bar(
            x=x_data,
            y=y_data,
            name=plot_config.y_label,
            marker_color=plot_config.color,
            opacity=plot_config.opacity
        ))
        
        self._apply_layout(fig, plot_config)
        return fig
    
    def create_histogram(self, data: List[Any], plot_config: PlotConfig) -> go.Figure:
        """
        Create a histogram with the given data and configuration.
        
        Args:
            data: Data to create histogram from
            plot_config: Configuration for the plot
            
        Returns:
            Configured plotly figure
        """
        fig = go.Figure()
        
        fig.add_trace(go.Histogram(
            x=data,
            name=plot_config.y_label,
            marker_color=plot_config.color,
            opacity=plot_config.opacity
        ))
        
        self._apply_layout(fig, plot_config)
        return fig
    
    def create_box_plot(self, data: List[Any], plot_config: PlotConfig) -> go.Figure:
        """
        Create a box plot with the given data and configuration.
        
        Args:
            data: Data to create box plot from
            plot_config: Configuration for the plot
            
        Returns:
            Configured plotly figure
        """
        fig = go.Figure()
        
        fig.add_trace(go.Box(
            y=data,
            name=plot_config.y_label,
            marker_color=plot_config.color,
            opacity=plot_config.opacity
        ))
        
        self._apply_layout(fig, plot_config)
        return fig
    
    def _apply_layout(self, fig: go.Figure, plot_config: PlotConfig) -> None:
        """Apply consistent layout configuration to a figure."""
        fig.update_layout(
            title=plot_config.title,
            xaxis_title=plot_config.x_label,
            yaxis_title=plot_config.y_label,
            template=self.config.template,
            width=self.config.width,
            height=self.config.height,
            margin=self.config.margin,
            showlegend=True,
            legend=dict(
                yanchor="top",
                y=0.99,
                xanchor="right",
                x=0.99
            )
        )

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

    @staticmethod
    def create_comparison_scatter(data_container_in, data_container_out, val_sig_map_in):
        """
        Create comparison scatter plots showing intersections and differences between input and output data.
        
        Parameters:
        - data_container_in: Dictionary containing input data
        - data_container_out: Dictionary containing output data
        - val_sig_map_in: Dictionary mapping signal identifiers
        
        Returns:
        - figs: Dictionary of Plotly figure objects
        """
        figs = {}
        unique_keys = set(data_container_in.keys()).union(set(data_container_out.keys()))
        inp_values = list(data_container_in.values())[0]
        
        for j in range(len(inp_values)):
            for k in range(len(inp_values[j])):
                fig_id = f"fig{j}_{k}"
                fig = go.Figure()
                
                for key, input_data, output_data in zip(unique_keys, data_container_in.values(), data_container_out.values()):
                    data_values = input_data[j][k]
                    data_out_values = output_data[j][k]
                    intersection = set(data_values).intersection(set(data_out_values))

                    # Plot intersection points
                    if intersection:
                        fig.add_trace(go.Scatter(
                            x=[key] * len(intersection),
                            y=list(intersection),
                            mode='markers',
                            name=f'Intersec {val_sig_map_in[f"{j}_{k}"]}',
                            marker=dict(color='purple', size=10)
                        ))

                    # Plot unique data points from input
                    unique_data_values = set(data_values) - intersection
                    if unique_data_values:
                        fig.add_trace(go.Scatter(
                            x=[key] * len(unique_data_values),
                            y=list(unique_data_values),
                            mode='markers',
                            name=f'Data {val_sig_map_in[f"{j}_{k}"]}',
                            marker=dict(color='blue')
                        ))

                    # Plot unique data points from output
                    unique_data_out_values = set(data_out_values) - intersection
                    if unique_data_out_values:
                        fig.add_trace(go.Scatter(
                            x=[key] * len(unique_data_out_values),
                            y=list(unique_data_out_values),
                            mode='markers',
                            name=f'Data Out {val_sig_map_in[f"{j}_{k}"]}',
                            marker=dict(color='red')
                        ))

                # Update layout for each figure
                fig.update_layout(
                    title=f'Scatter Plot with {val_sig_map_in[f"{j}_{k}"]}',
                    xaxis_title='ScanIndex',
                    yaxis_title='Values',
                    legend_title='Legend'
                )
                figs[fig_id] = fig
                
        return figs
