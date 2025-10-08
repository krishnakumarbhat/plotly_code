from typing import Dict, List, Any, Optional
from dataclasses import dataclass
import plotly.graph_objects as go
import pandas as pd
import numpy as np

from KPI.detection_matching_kpi import KpiDataModel
from InteractivePlot.e_presentation_layer.plotly_visualization import PlotlyCharts
from InteractivePlot.e_presentation_layer.html_generator import HtmlGenerator

@dataclass
class PlotConfig:
    """Configuration for plot generation"""
    title: str
    x_label: str
    y_label: str
    plot_type: str = "scatter"
    color: str = "blue"
    opacity: float = 0.6

class DataPreprocessor:
    """Handles data preprocessing and transformation for visualization"""
    
    @staticmethod
    def handle_missing_data(data: List[Any]) -> List[Any]:
        """
        Handle missing or invalid data points.
        
        Args:
            data: Raw data list that may contain missing values
            
        Returns:
            List with properly handled missing values
        """
        return [np.nan if x is None or x == [] else x for x in data]
    
    @staticmethod
    def create_dataframe(x_data: List[Any], y_data: List[Any], 
                        x_label: str, y_label: str) -> pd.DataFrame:
        """
        Create a pandas DataFrame from x and y data series.
        
        Args:
            x_data: Data for x-axis
            y_data: Data for y-axis
            x_label: Label for x-axis
            y_label: Label for y-axis
            
        Returns:
            DataFrame with processed data
        """
        df = pd.DataFrame({
            x_label: DataPreprocessor.handle_missing_data(x_data),
            y_label: DataPreprocessor.handle_missing_data(y_data)
        })
        return df.dropna()  # Remove rows with missing values

class DataPrep:
    """
    Manages data preparation and visualization pipeline by coordinating
    data preprocessing, KPI calculations, and plot generation.
    """
    
    def __init__(self, data_container_in: Dict[str, List[Any]], 
                 val_sig_map_in: Dict[str, str],
                 sig_val_map_in: Dict[str, Any],
                 data_container_out: Dict[str, List[Any]],
                 val_sig_map_out: Dict[str, str],
                 sig_val_map_out: Dict[str, Any],
                 html_name: str):
        """
        Initialize the data preparation pipeline.
        
        Args:
            data_container_in: Input data container
            val_sig_map_in: Input value-to-signal mapping
            sig_val_map_in: Input signal-to-value mapping
            data_container_out: Output data container
            val_sig_map_out: Output value-to-signal mapping
            sig_val_map_out: Output signal-to-value mapping
            html_name: Name for generated HTML visualization
        """
        self.input_data = {
            'container': data_container_in,
            'val_to_sig': val_sig_map_in,
            'sig_to_val': sig_val_map_in
        }
        
        self.output_data = {
            'container': data_container_out,
            'val_to_sig': val_sig_map_out,
            'sig_to_val': sig_val_map_out
        }
        
        self.html_name = html_name
        self.preprocessor = DataPreprocessor()
        
        # Initialize visualization components
        self.kpi_analyzer = KpiDataModel(
            data_container_in, val_sig_map_in, sig_val_map_in,
            data_container_out, val_sig_map_out, sig_val_map_out
        )
        
        # Generate plots and create visualization
        self.plot_collection = self.generate_plots()
        HtmlGenerator(self.plot_collection, self.kpi_analyzer, self.html_name)
    
    def generate_plots(self) -> Dict[str, List[go.Figure]]:
        """
        Generate all visualization plots by processing input and output data.
        
        Returns:
            Dictionary mapping plot categories to lists of plotly figures
        """
        plots = {}
        
        # Process input data plots
        input_plots = self._generate_data_plots(
            self.input_data['container'],
            self.input_data['val_to_sig'],
            "Input Signals"
        )
        if input_plots:
            plots['input'] = input_plots
            
        # Process output data plots
        output_plots = self._generate_data_plots(
            self.output_data['container'],
            self.output_data['val_to_sig'],
            "Output Signals"
        )
        if output_plots:
            plots['output'] = output_plots
            
        return plots
    
    def _generate_data_plots(self, data_container: Dict[str, List[Any]],
                           val_sig_map: Dict[str, str],
                           category: str) -> List[go.Figure]:
        """
        Generate plots for a specific data category.
        
        Args:
            data_container: Container with raw data
            val_sig_map: Value to signal mapping
            category: Category name for the plots
            
        Returns:
            List of generated plotly figures
        """
        plots = []
        plotter = PlotlyCharts()
        
        for val_key, signal_name in val_sig_map.items():
            if not signal_name:
                continue
                
            config = PlotConfig(
                title=f"{category}: {signal_name}",
                x_label="Time",
                y_label=signal_name
            )
            
            # Create time series data
            time_points = range(len(data_container.get(val_key, [])))
            signal_data = data_container.get(val_key, [])
            
            # Preprocess data
            df = self.preprocessor.create_dataframe(
                time_points, signal_data,
                config.x_label, config.y_label
            )
            
            if not df.empty:
                plot = plotter.create_scatter_plot(
                    df[config.x_label],
                    df[config.y_label],
                    config
                )
                plots.append(plot)
                
        return plots
