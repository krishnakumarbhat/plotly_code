"""
HDF file reader for Interactive Plot System.

This module provides functionality to read data from HDF files.
"""
import os
import h5py
import numpy as np
from typing import Dict, List, Any, Optional

from InteractivePlot.interfaces.base_interfaces import IDataReader, IProgressReporter


class BaseHdfReader(IDataReader):
    """Base class for HDF file readers."""
    
    def __init__(self, progress_reporter: Optional[IProgressReporter] = None):
        """
        Initialize the HDF reader.
        
        Args:
            progress_reporter: Optional progress reporter
        """
        self.progress_reporter = progress_reporter
        
    def read_sensors(self, file_path: str) -> List[str]:
        """
        Read the list of sensors from an HDF file.
        
        Args:
            file_path: Path to the HDF file
            
        Returns:
            List of sensor names
        """
        if not os.path.exists(file_path):
            raise FileNotFoundError(f"HDF file not found: {file_path}")
            
        try:
            with h5py.File(file_path, 'r') as f:
                # For simplicity, let's assume sensors are top-level keys that don't start with '_'
                # In a real implementation, this would need to be adjusted based on the actual file structure
                sensors = [key for key in f.keys() if not key.startswith('_')]
                return sensors
        except Exception as e:
            raise ValueError(f"Error reading sensors from HDF file: {e}")
    
    def read_data(self, file_path: str) -> Dict[str, Any]:
        """
        Read data from an HDF file.
        
        Args:
            file_path: Path to the HDF file
            
        Returns:
            Dictionary of data
        """
        if not os.path.exists(file_path):
            raise FileNotFoundError(f"HDF file not found: {file_path}")
            
        try:
            data = {}
            
            with h5py.File(file_path, 'r') as f:
                self._read_groups_recursive(f, data)
                
            return data
        except Exception as e:
            raise ValueError(f"Error reading data from HDF file: {e}")
    
    def _read_groups_recursive(self, group, data, path=""):
        """
        Recursively read groups and datasets from an HDF file.
        
        Args:
            group: HDF group
            data: Dictionary to store data
            path: Current path in the HDF file
        """
        for key in group.keys():
            item = group[key]
            new_path = f"{path}/{key}" if path else key
            
            if isinstance(item, h5py.Group):
                data[key] = {}
                self._read_groups_recursive(item, data[key], new_path)
            else:
                # Check if the dataset is a reference
                if isinstance(item, h5py.Reference):
                    data[key] = item
                else:
                    # Convert numpy arrays to Python lists for better serialization
                    array_data = item[()]
                    if isinstance(array_data, np.ndarray):
                        data[key] = array_data.tolist()
                    else:
                        data[key] = array_data


class AllsensorHdfReader(BaseHdfReader):
    """Reader for all-sensor HDF files."""
    
    def read_sensors(self, file_path: str) -> List[str]:
        """
        Read the list of sensors from an all-sensor HDF file.
        
        Args:
            file_path: Path to the HDF file
            
        Returns:
            List of sensor names
        """
        # For all-sensor format, we can use the base implementation
        return super().read_sensors(file_path)
    
    def read_data(self, file_path: str) -> Dict[str, Any]:
        """
        Read data from an all-sensor HDF file.
        
        Args:
            file_path: Path to the HDF file
            
        Returns:
            Dictionary of data
        """
        # For all-sensor format, we can use the base implementation
        return super().read_data(file_path)


class PersensorHdfReader(BaseHdfReader):
    """Reader for per-sensor HDF files."""
    
    def read_sensors(self, file_path: str) -> List[str]:
        """
        Read the list of sensors from a per-sensor HDF file.
        
        Args:
            file_path: Path to the HDF file
            
        Returns:
            List of sensor names
        """
        # For per-sensor format, we might need a custom implementation
        # For simplicity, we'll assume the file name contains the sensor name
        filename = os.path.basename(file_path)
        # Extract sensor name from filename (e.g., "data_FL.hdf" -> "FL")
        if '_' in filename:
            sensor = filename.split('_')[1].split('.')[0]
            return [sensor]
        else:
            # If we can't determine the sensor from the filename, use the base implementation
            return super().read_sensors(file_path)
    
    def read_data(self, file_path: str) -> Dict[str, Any]:
        """
        Read data from a per-sensor HDF file.
        
        Args:
            file_path: Path to the HDF file
            
        Returns:
            Dictionary of data
        """
        # For per-sensor format, we can still use the base implementation
        return super().read_data(file_path)


class HdfReaderFactory:
    """Factory for creating appropriate HDF reader instances."""
    
    @staticmethod
    def create_reader(hdf_file_type: str, progress_reporter: Optional[IProgressReporter] = None) -> IDataReader:
        """
        Create an HDF reader based on the file type.
        
        Args:
            hdf_file_type: Type of HDF file
            progress_reporter: Optional progress reporter
            
        Returns:
            An HDF reader instance
        """
        if hdf_file_type == "HDF_WITH_ALLSENSOR":
            return AllsensorHdfReader(progress_reporter)
        elif hdf_file_type == "HDF_PER_SENSOR":
            return PersensorHdfReader(progress_reporter)
        else:
            raise ValueError(f"Invalid HDF file type: {hdf_file_type}") 