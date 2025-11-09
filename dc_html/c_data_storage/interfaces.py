from abc import ABC, abstractmethod
from typing import Dict, List, Any, Protocol
import numpy as np

class IDataStorage(ABC):
    """Interface for data storage operations."""
    
    @abstractmethod
    def initialize(self, scan_index: List[int], sensor: str, stream: str) -> None:
        """Initialize the data container with scan indices."""
        pass
    
    @abstractmethod
    def set_value(self, dataset: Any, signal_name: str, grp_name: str) -> str:
        """Set a value in the storage with group relationship."""
        pass
    
    @abstractmethod
    def clear(self) -> None:
        """Clear all stored data and reset counters."""
        pass

class IDataExtractor(ABC):
    """Interface for data extraction operations."""
    
    @abstractmethod
    def extract_data(self, hdf_file_path: str, dataset_path: str, signal_names: List[str]) -> Dict[str, np.ndarray]:
        """Extract data from HDF file for specified signals."""
        pass
    
    @abstractmethod
    def validate_data(self, data: Dict[str, np.ndarray]) -> bool:
        """Validate extracted data."""
        pass

class IDataFormatter(ABC):
    """Interface for data formatting operations."""
    
    @abstractmethod
    def format_data(self, storage: IDataStorage, output_file: str, header: str) -> bool:
        """Format and save data to file."""
        pass

class ILogger(Protocol):
    """Protocol for logging operations."""
    
    def info(self, message: str) -> None:
        """Log info message."""
        ...
    
    def error(self, message: str) -> None:
        """Log error message."""
        ...
    
    def warning(self, message: str) -> None:
        """Log warning message."""
        ...

class IConfiguration(ABC):
    """Interface for configuration management."""
    
    @abstractmethod
    def get_dataset_config(self, source: str) -> Dict[str, Dict]:
        """Get dataset configuration for a specific source."""
        pass
    
    @abstractmethod
    def get_output_config(self) -> Dict[str, str]:
        """Get output configuration."""
        pass 