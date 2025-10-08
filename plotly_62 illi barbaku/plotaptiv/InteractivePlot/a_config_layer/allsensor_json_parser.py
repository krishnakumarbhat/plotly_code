from typing import Dict, List, Any
import json
from .json_parser_factory import JsonParser

class AllSensorJsonParser(JsonParser):
    """Parser for JSON files containing data for all sensors"""
    
    def __init__(self, config_file: str):
        """
        Initialize parser with configuration file path.
        
        Args:
            config_file: Path to JSON configuration file
        """
        self.config_file = config_file
        self.config_data: Dict[str, Any] = {}
        self._load_config()
    
    def _load_config(self) -> None:
        """Load and validate configuration data from file."""
        try:
            with open(self.config_file, 'r') as f:
                self.config_data = json.load(f)
        except (json.JSONDecodeError, FileNotFoundError) as e:
            raise ValueError(f"Failed to load configuration file: {str(e)}")
    
    def parse(self, json_data: Dict) -> Dict[str, Any]:
        """
        Parse JSON data according to all-sensor format requirements.
        
        Args:
            json_data: Raw JSON data to parse
            
        Returns:
            Parsed and validated sensor data
            
        Raises:
            ValueError: If required fields are missing or invalid
        """
        if not isinstance(json_data, dict):
            raise ValueError("Input JSON data must be a dictionary")
            
        parsed_data = {
            'sensors': self._parse_sensors(json_data.get('sensors', {})),
            'metadata': self._parse_metadata(json_data.get('metadata', {}))
        }
        
        return parsed_data
    
    def _parse_sensors(self, sensor_data: Dict) -> Dict[str, Any]:
        """Parse and validate sensor configuration data."""
        if not sensor_data:
            raise ValueError("No sensor data found in configuration")
            
        parsed_sensors = {}
        for sensor_id, config in sensor_data.items():
            parsed_sensors[sensor_id] = {
                'name': config.get('name', ''),
                'type': config.get('type', ''),
                'parameters': config.get('parameters', {}),
                'enabled': config.get('enabled', True)
            }
            
        return parsed_sensors
    
    def _parse_metadata(self, metadata: Dict) -> Dict[str, Any]:
        """Parse and validate metadata configuration."""
        return {
            'version': metadata.get('version', '1.0'),
            'description': metadata.get('description', ''),
            'created_at': metadata.get('created_at', ''),
            'updated_at': metadata.get('updated_at', '')
        }
