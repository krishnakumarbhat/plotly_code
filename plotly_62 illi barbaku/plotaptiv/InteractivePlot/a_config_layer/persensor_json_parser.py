from typing import Dict, List, Any, Optional
import json
from pathlib import Path
from .json_parser_factory import JsonParser

class SensorConfig:
    """Configuration data for a single sensor"""
    
    def __init__(self, config_data: Dict[str, Any]):
        """
        Initialize sensor configuration.
        
        Args:
            config_data: Raw configuration data for the sensor
        """
        self.sensor_id = config_data.get('id', '')
        self.name = config_data.get('name', '')
        self.type = config_data.get('type', '')
        self.input_file = config_data.get('input_file', '')
        self.output_file = config_data.get('output_file', '')
        self.parameters = config_data.get('parameters', {})
        self.enabled = config_data.get('enabled', True)

class PerSensorJsonParser(JsonParser):
    """Parser for JSON files containing individual sensor configurations"""
    
    def __init__(self, config_path: str):
        """
        Initialize parser with configuration file path.
        
        Args:
            config_path: Path to JSON configuration file
        """
        self.config_path = Path(config_path)
        self.sensors: Dict[str, SensorConfig] = {}
        self.metadata: Dict[str, Any] = {}
        
    def parse(self, json_data: Dict) -> Dict[str, Any]:
        """
        Parse JSON data according to per-sensor format requirements.
        
        Args:
            json_data: Raw JSON data to parse
            
        Returns:
            Parsed and validated configuration data
            
        Raises:
            ValueError: If required fields are missing or invalid
        """
        try:
            # Parse metadata
            self.metadata = self._parse_metadata(json_data.get('metadata', {}))
            
            # Parse sensor configurations
            sensor_configs = json_data.get('sensors', {})
            if not sensor_configs:
                raise ValueError("No sensor configurations found")
                
            for sensor_id, config in sensor_configs.items():
                self.sensors[sensor_id] = SensorConfig(config)
            
            return self._build_output()
            
        except Exception as e:
            raise ValueError(f"Failed to parse sensor configuration: {str(e)}")
    
    def _parse_metadata(self, metadata: Dict) -> Dict[str, Any]:
        """
        Parse and validate metadata configuration.
        
        Args:
            metadata: Raw metadata dictionary
            
        Returns:
            Validated metadata dictionary
        """
        return {
            'version': metadata.get('version', '1.0'),
            'description': metadata.get('description', ''),
            'created_at': metadata.get('created_at', ''),
            'updated_at': metadata.get('updated_at', ''),
            'total_sensors': len(metadata.get('sensor_ids', []))
        }
    
    def _build_output(self) -> Dict[str, Any]:
        """
        Build final output dictionary from parsed data.
        
        Returns:
            Dictionary containing parsed configuration data
        """
        return {
            'metadata': self.metadata,
            'sensors': {
                sensor_id: {
                    'id': sensor.sensor_id,
                    'name': sensor.name,
                    'type': sensor.type,
                    'input_file': sensor.input_file,
                    'output_file': sensor.output_file,
                    'parameters': sensor.parameters,
                    'enabled': sensor.enabled
                }
                for sensor_id, sensor in self.sensors.items()
            }
        }
    
    def get_sensor_config(self, sensor_id: str) -> Optional[SensorConfig]:
        """
        Get configuration for a specific sensor.
        
        Args:
            sensor_id: ID of the sensor
            
        Returns:
            Sensor configuration if found, None otherwise
        """
        return self.sensors.get(sensor_id)
    
    def get_enabled_sensors(self) -> List[SensorConfig]:
        """
        Get list of all enabled sensors.
        
        Returns:
            List of enabled sensor configurations
        """
        return [sensor for sensor in self.sensors.values() if sensor.enabled]
    
    def get_input_output_mapping(self) -> Dict[str, str]:
        """
        Get mapping of input files to output files.
        
        Returns:
            Dictionary mapping input file paths to output file paths
        """
        return {
            sensor.input_file: sensor.output_file
            for sensor in self.sensors.values()
            if sensor.enabled and sensor.input_file and sensor.output_file
        }
