from dataclasses import dataclass
from typing import Dict, List, Any
from InteractivePlot.d_business_layer.data_prep import DataPrep
# from KPI.detection_matching_kpi import KPI_Detector

@dataclass
class SignalContainer:
    """Container for signal data and mappings"""
    data: Dict[str, List[Any]]  # Raw data container
    value_to_signal: Dict[str, str]  # Value to signal name mapping
    signal_to_value: Dict[str, Any]  # Signal name to value mapping

@dataclass
class DataModel:
    """
    Core business model that manages data processing and visualization pipeline.
    Handles both input and output data streams while maintaining their relationships.
    """
    
    def __init__(self, 
                 input_data: SignalContainer,
                 output_data: SignalContainer,
                 visualization_name: str):
        """
        Initialize the data model with input/output containers and visualization settings.
        
        Args:
            input_data: Container for input signal data and mappings
            output_data: Container for output signal data and mappings
            visualization_name: Name for the generated visualization file
        """
        self.input = input_data
        self.output = output_data
        self.visualization_name = visualization_name
        
        # Initialize data processing pipeline
        self.processor = DataPrep(
            data_container_in=self.input.data,
            val_sig_map_in=self.input.value_to_signal,
            sig_val_map_in=self.input.signal_to_value,
            data_container_out=self.output.data,
            val_sig_map_out=self.output.value_to_signal,
            sig_val_map_out=self.output.signal_to_value,
            html_name=self.visualization_name
        )
#         self.kpi_detector = RadarDataProvider:
# (
#             data_container_in=data_container_in,
#             val_sig_map_in=val_sig_map_in,
#             sig_val_map_in=sig_val_map_in,
#             data_container_out=data_container_out,
#             val_sig_map_out=val_sig_map_out,          
#             sig_val_map_out=sig_val_map_out,
#             html_name=html_name
#         )