import os
import logging
from typing import Dict, List
from c_data_storage.interfaces import IDataStorage, IDataExtractor, IDataFormatter, IConfiguration, ILogger
from c_data_storage.data_model_storage import DataModelStorage
from c_data_storage.config_storage import ConfigurationManager
from d_business_layer.data_extractor import HDFDataExtractor
from d_business_layer.data_formatter import DataModelStorageFormatter
from d_business_layer.utils import time_taken

class DataDumper:
    """
    Main data dumper class that orchestrates the data extraction and formatting process.
    Follows SOLID principles:
    - Single Responsibility: Orchestrates the process
    - Open/Closed: Open for extension through interfaces
    - Dependency Inversion: Depends on abstractions, not concretions
    """
    
    def __init__(
        self, 
        hdf_file_path: str, 
        output_dir: str = "html",
        config: IConfiguration = None,
        extractor: IDataExtractor = None,
        formatter: IDataFormatter = None,
        logger: ILogger = None
    ):
        """
        Initialize the DataDumper with dependency injection.
        
        Args:
            hdf_file_path: Path to the HDF file
            output_dir: Directory to save output files
            config: Configuration manager (injected dependency)
            extractor: Data extractor (injected dependency)
            formatter: Data formatter (injected dependency)
            logger: Logger instance (injected dependency)
        """
        self.hdf_file_path = hdf_file_path
        self.output_dir = output_dir
        
        # Dependency injection with defaults
        self.config = config or ConfigurationManager()
        self.logger = logger or self._create_default_logger()
        self.extractor = extractor or HDFDataExtractor(self.logger)
        self.formatter = formatter or DataModelStorageFormatter(self.logger)
        
        # Ensure output directory exists
        os.makedirs(output_dir, exist_ok=True)
        
        # Initialize storage instances
        self.tracker_storage = DataModelStorage()
        self.osi_storage = DataModelStorage()
    
    def _create_default_logger(self) -> ILogger:
        """Create a default logger instance."""
        logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
        return logging.getLogger(__name__)
    
    @time_taken
    def dump_tracker_data(self) -> bool:
        """
        Dump Tracker_Information/OLP datasets using Strategy pattern.
        
        Returns:
            bool: True if successful, False otherwise
        """
        return self._dump_data_source("Tracker_Information", "OLP", self.tracker_storage, "Track.txt")
    
    @time_taken
    def dump_osi_data(self) -> bool:
        """
        Dump OSI_Ground_Truth/Object datasets using Strategy pattern.
        
        Returns:
            bool: True if successful, False otherwise
        """
        return self._dump_data_source("OSI_Ground_Truth", "Object", self.osi_storage, "OSI track.txt")
    
    def _dump_data_source(self, source: str, stream: str, storage: IDataStorage, output_filename: str) -> bool:
        """
        Generic method to dump data from any source using Strategy pattern.
        
        Args:
            source: Data source name (e.g., "Tracker_Information", "OSI_Ground_Truth")
            stream: Stream name (e.g., "OLP", "Object")
            storage: Data storage instance
            output_filename: Output filename
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            # Get configuration for this source
            dataset_config = self.config.get_dataset_config(source)
            if not dataset_config:
                self.logger.warning(f"No configuration found for source: {source}")
                return False
            
            signal_names = list(dataset_config.keys())
            dataset_path = f"{source}/{stream}"
            
            # Extract data
            extracted_data = self.extractor.extract_data(
                self.hdf_file_path, 
                dataset_path, 
                signal_names
            )
            
            if not extracted_data:
                self.logger.warning(f"No data extracted for source: {source}")
                return False
            
            # Validate data
            if not self.extractor.validate_data(extracted_data):
                self.logger.error(f"Data validation failed for source: {source}")
                return False
            
            # Initialize storage
            first_dataset_shape = self.extractor.get_first_dataset_shape(self.hdf_file_path, dataset_path)
            if not first_dataset_shape:
                self.logger.error(f"Could not determine dataset shape for source: {source}")
                return False
            
            scan_indices = list(range(first_dataset_shape[0]))
            storage.initialize(scan_indices, source, stream)
            storage.init_parent(stream)
            
            # Store data
            for signal_name, dataset in extracted_data.items():
                storage.set_value(dataset, signal_name, stream)
            
            # Format and save data
            output_file = os.path.join(self.output_dir, output_filename)
            header = f"{source} - {stream}"
            
            success = self.formatter.format_data(storage, output_file, header)
            
            if success:
                self.logger.info(f"Successfully dumped {source}/{stream} data to {output_file}")
            else:
                self.logger.error(f"Failed to format data for {source}/{stream}")
            
            return success
            
        except Exception as e:
            self.logger.error(f"Error dumping {source} data: {e}")
            return False
    
    def dump_all_data(self) -> Dict[str, bool]:
        """
        Dump all configured data sources.
        
        Returns:
            Dict[str, bool]: Dictionary with results for each dump operation
        """
        results = {}
        
        # Get all available sources from configuration
        sources = self.config.get_all_sources()
        
        for source in sources:
            # Determine stream and output filename based on source
            if source == "Tracker_Information":
                results['tracker'] = self.dump_tracker_data()
            elif source == "OSI_Ground_Truth":
                results['osi'] = self.dump_osi_data()
            else:
                # Generic handling for new sources
                stream = self._get_stream_for_source(source)
                output_filename = f"{source.replace('_', ' ')}.txt"
                storage = DataModelStorage()
                results[source.lower()] = self._dump_data_source(source, stream, storage, output_filename)
        
        return results
    
    def _get_stream_for_source(self, source: str) -> str:
        """
        Get the appropriate stream name for a data source.
        
        Args:
            source: Data source name
            
        Returns:
            str: Stream name
        """
        # Default mapping - can be extended for new sources
        stream_mapping = {
            "Tracker_Information": "OLP",
            "OSI_Ground_Truth": "Object"
        }
        return stream_mapping.get(source, "Default")
    
    def get_storage_info(self) -> Dict[str, Dict]:
        """
        Get information about the storage instances.
        
        Returns:
            Dict containing storage information
        """
        return {
            'tracker': {
                'scan_indices': len(self.tracker_storage.get_data_container()),
                'signals': len(self.tracker_storage.get_signal_to_value()),
                'parent_counter': self.tracker_storage.get_parent_counter(),
                'child_counter': self.tracker_storage.get_child_counter()
            },
            'osi': {
                'scan_indices': len(self.osi_storage.get_data_container()),
                'signals': len(self.osi_storage.get_signal_to_value()),
                'parent_counter': self.osi_storage.get_parent_counter(),
                'child_counter': self.osi_storage.get_child_counter()
            }
        } 