import h5py
import numpy as np
import os
from typing import Dict, List
from c_data_storage.interfaces import IDataExtractor, ILogger

class HDFDataExtractor(IDataExtractor):
    """
    HDF data extractor implementation.
    Follows Single Responsibility Principle by focusing only on data extraction from HDF files.
    """
    
    def __init__(self, logger: ILogger):
        """
        Initialize the HDF data extractor.
        
        Args:
            logger: Logger instance for logging operations
        """
        self.logger = logger
    
    def extract_data(self, hdf_file_path: str, dataset_path: str, signal_names: List[str]) -> Dict[str, np.ndarray]:
        """
        Extract data from HDF file for specified signals.
        
        Args:
            hdf_file_path: Path to the HDF file
            dataset_path: Path to the dataset within the HDF file
            signal_names: List of signal names to extract
            
        Returns:
            Dictionary mapping signal names to their data arrays
        """
        extracted_data = {}
        
        try:
            with h5py.File(hdf_file_path, 'r') as f:
                # Navigate to the dataset path
                if dataset_path not in f:
                    self.logger.warning(f"Dataset path '{dataset_path}' not found in HDF file")
                    return extracted_data
                
                dataset_group = f[dataset_path]
                
                # Extract each signal
                for signal_name in signal_names:
                    if signal_name in dataset_group:
                        data = dataset_group[signal_name][()]
                        extracted_data[signal_name] = data
                        self.logger.info(f"Extracted signal '{signal_name}' with shape {data.shape}")
                    else:
                        self.logger.warning(f"Signal '{signal_name}' not found in dataset '{dataset_path}'")
                        
        except Exception as e:
            self.logger.error(f"Error extracting data from HDF file: {e}")
            raise
        
        return extracted_data
    
    def validate_data(self, data: Dict[str, np.ndarray]) -> bool:
        """
        Validate extracted data.
        
        Args:
            data: Dictionary of extracted data arrays
            
        Returns:
            True if data is valid, False otherwise
        """
        if not data:
            self.logger.warning("No data extracted")
            return False
        
        # Check if all arrays have the same first dimension (time dimension)
        shapes = [arr.shape[0] for arr in data.values() if len(arr.shape) > 0]
        if not shapes:
            self.logger.warning("No valid arrays found in extracted data")
            return False
        
        if len(set(shapes)) > 1:
            self.logger.warning(f"Inconsistent time dimensions: {shapes}")
            return False
        
        self.logger.info(f"Data validation passed. Time dimension: {shapes[0]}")
        return True
    
    def get_first_dataset_shape(self, hdf_file_path: str, dataset_path: str) -> tuple:
        """
        Get the shape of the first dataset to determine scan indices.
        
        Args:
            hdf_file_path: Path to the HDF file
            dataset_path: Path to the dataset within the HDF file
            
        Returns:
            Shape of the first dataset found
        """
        try:
            with h5py.File(hdf_file_path, 'r') as f:
                if dataset_path not in f:
                    return ()
                
                dataset_group = f[dataset_path]
                for signal_name in dataset_group.keys():
                    return dataset_group[signal_name].shape
                    
        except Exception as e:
            self.logger.error(f"Error getting dataset shape: {e}")
        
        return () 