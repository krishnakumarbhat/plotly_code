from c_data_storage.interfaces import IConfiguration
from typing import Dict

class ConfigurationManager(IConfiguration):
    """
    Configuration manager for data extraction operations.
    Follows Single Responsibility Principle by focusing only on configuration management.
    """
    
    def __init__(self):
        # Dictionary of signal patterns for data extraction
        self._dataset_config = {
            "Tracker_Information": {
                "vcs_accel_x": {},
                "vcs_accel_y": {},
                "vcs_pos_x": {},
                "vcs_pos_y": {},
                "vcs_vel_x": {},
                "vcs_vel_y": {}
            },
            "OSI_Ground_Truth": {
                "length": {},
                "speed": {},
                "vcs_lat_accel": {},
                "vcs_lat_posn": {},
                "vcs_lat_vel": {},
                "vcs_long_accel": {},
                "vcs_long_posn": {},
                "vcs_long_vel": {}
            }
        }
        
        # Output configuration
        self._output_config = {
            "tracker_output": "Track.txt",
            "osi_output": "OSI track.txt",
            "output_dir": "html"
        }
    
    def get_dataset_config(self, source: str) -> Dict[str, Dict]:
        """
        Get dataset configuration for a specific source.
        
        Args:
            source: The data source name (e.g., "Tracker_Information", "OSI_Ground_Truth")
            
        Returns:
            Dictionary containing signal configurations for the source
        """
        return self._dataset_config.get(source, {})
    
    def get_output_config(self) -> Dict[str, str]:
        """
        Get output configuration.
        
        Returns:
            Dictionary containing output file names and directory
        """
        return self._output_config.copy()
    
    def get_all_sources(self) -> list:
        """
        Get all available data sources.
        
        Returns:
            List of available data source names
        """
        return list(self._dataset_config.keys())
    
    def get_signals_for_source(self, source: str) -> list:
        """
        Get all signals for a specific source.
        
        Args:
            source: The data source name
            
        Returns:
            List of signal names for the source
        """
        config = self.get_dataset_config(source)
        return list(config.keys())

# Legacy support - keep the old variable name for backward compatibility
Gen7V1_V2 = ConfigurationManager()._dataset_config
