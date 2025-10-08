"""
Base interfaces for the Interactive Plot System.
This module defines the core abstractions for the system following SOLID principles.
"""
from abc import ABC, abstractmethod
from typing import Dict, List, Any, Optional, Tuple, Set
import h5py
import numpy as np
import os
import pandas as pd


# --- File I/O Interfaces ---

class IHdfReader(ABC):
    """Interface for reading HDF5 files."""
    
    @abstractmethod
    def read_sensors(self, file_path: str) -> Set[str]:
        """
        Read available sensors from HDF5 file.
        
        Args:
            file_path: Path to the HDF5 file
            
        Returns:
            Set of sensor names
        """
        pass
    
    @abstractmethod
    def read_data(self, file_path: str, sensor: Optional[str] = None) -> Dict[str, Any]:
        """
        Read data from HDF5 file.
        
        Args:
            file_path: Path to the HDF5 file
            sensor: Optional sensor name to filter data
            
        Returns:
            Dictionary containing the sensor data
        """
        pass


# --- Data Processing Interfaces ---

class IDataProcessor(ABC):
    """Interface for processing sensor data."""
    
    @abstractmethod
    def process(self, input_data: Dict[str, Any], output_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Process input and output data to prepare for visualization.
        
        Args:
            input_data: Dictionary containing input sensor data
            output_data: Dictionary containing output sensor data
            
        Returns:
            Processed data ready for visualization
        """
        pass


class ISensorProcessor(ABC):
    """Interface for processing individual sensor data."""
    
    @abstractmethod
    def process_sensor(self, 
                     sensor: str, 
                     input_data: Dict[str, Any], 
                     output_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Process data for a specific sensor.
        
        Args:
            sensor: Name of the sensor
            input_data: Dictionary containing input sensor data
            output_data: Dictionary containing output sensor data
            
        Returns:
            Processed data for the sensor
        """
        pass


# --- Visualization Interfaces ---

class IVisualizer(ABC):
    """Interface for data visualization."""
    
    @abstractmethod
    def create_visualization(self, processed_data: Dict[str, Any]) -> Any:
        """
        Create visualization from processed data.
        
        Args:
            processed_data: Dictionary containing processed data
            
        Returns:
            Visualization object (e.g., Plotly figure)
        """
        pass


class IHtmlReportGenerator(ABC):
    """Interface for generating HTML reports."""
    
    @abstractmethod
    def generate_report(self, 
                       visualizations: Dict[str, Any], 
                       output_file: str,
                       output_dir: str) -> str:
        """
        Generate an HTML report with visualizations.
        
        Args:
            visualizations: Dictionary of visualization objects
            output_file: Name of the output file
            output_dir: Output directory
            
        Returns:
            Path to the generated HTML file
        """
        pass


# --- Progress Reporting Interfaces ---

class IProgressObserver(ABC):
    """Interface for progress observers."""
    
    @abstractmethod
    def update(self, progress: float, message: str) -> None:
        """
        Update the observer with progress information.
        
        Args:
            progress: Progress percentage (0-100)
            message: Progress message
        """
        pass


class IProgressReporter(ABC):
    """Interface for progress reporters."""
    
    @abstractmethod
    def register_observer(self, observer: IProgressObserver) -> None:
        """
        Register a progress observer.
        
        Args:
            observer: The progress observer to register
        """
        pass
    
    @abstractmethod
    def unregister_observer(self, observer: IProgressObserver) -> None:
        """
        Unregister a progress observer.
        
        Args:
            observer: The progress observer to unregister
        """
        pass
    
    @abstractmethod
    def notify_observers(self, progress: float, message: str) -> None:
        """
        Notify all registered observers of progress.
        
        Args:
            progress: Progress percentage (0-100)
            message: Progress message
        """
        pass


# --- Factory Interfaces ---

class IParserFactory(ABC):
    """Interface for parser factories."""
    
    @abstractmethod
    def create_parser(self, file_type: str, config_file: str) -> Any:
        """
        Create a parser for the specified file type.
        
        Args:
            file_type: Type of file to parse
            config_file: Configuration file path
            
        Returns:
            Parser object
        """
        pass


class IProcessorFactory(ABC):
    """Interface for processor factories."""
    
    @abstractmethod
    def create_processor(self, processor_type: str) -> IDataProcessor:
        """
        Create a processor of the specified type.
        
        Args:
            processor_type: Type of processor to create
            
        Returns:
            Processor object
        """
        pass


class IVisualizerFactory(ABC):
    """Interface for visualizer factories."""
    
    @abstractmethod
    def create_visualizer(self, visualization_type: str) -> IVisualizer:
        """
        Create a visualizer of the specified type.
        
        Args:
            visualization_type: Type of visualizer to create
            
        Returns:
            Visualizer object
        """
        pass


class IConfigParser(ABC):
    """Interface for configuration parsers."""
    
    @abstractmethod
    def parse(self) -> Any:
        """
        Parse the configuration.
        
        Returns:
            The parsed configuration
        """
        pass


class IInputOutputParser(ABC):
    """Interface for input-output parsers."""
    
    @abstractmethod
    def get_io_map(self) -> Dict[str, str]:
        """
        Get the mapping of input files to output files.
        
        Returns:
            Dictionary mapping input file paths to output file paths
        """
        pass


class IDataReader(ABC):
    """Interface for data readers."""
    
    @abstractmethod
    def read_sensors(self, file_path: str) -> List[str]:
        """
        Read the list of sensors from a file.
        
        Args:
            file_path: Path to the file
            
        Returns:
            List of sensor names
        """
        pass
    
    @abstractmethod
    def read_data(self, file_path: str) -> Dict[str, Any]:
        """
        Read data from a file.
        
        Args:
            file_path: Path to the file
            
        Returns:
            Dictionary of data
        """
        pass


class IReportGenerator(ABC):
    """Interface for report generators."""
    
    @abstractmethod
    def generate_report(self, visualizations: Dict[str, Any], output_file: str, output_dir: Optional[str] = None) -> str:
        """
        Generate a report.
        
        Args:
            visualizations: Dictionary of visualizations
            output_file: Name of the output file
            output_dir: Directory to save the report in
            
        Returns:
            Path to the generated report
        """
        pass 