# stream_manager.py
from typing import List, Dict, Set
import logging

class DataProcessor:
    """Manages stream selection and loading for memory optimization."""
    
    def __init__(self, sensor_list: List[str], available_streams: List[str]):
        """Initialize with available sensors and streams."""
        self.sensor_list = sensor_list
        self.available_streams = available_streams
        self.selected_streams: Dict[str, Set[str]] = {}
        
    def select_streams(self, sensor: str, streams: List[str]) -> None:
        """Select specific streams for a sensor to be loaded."""
        valid_streams = [s for s in streams if s in self.available_streams]
        if not valid_streams:
            logging.warning(f"No valid streams selected for sensor {sensor}")
            return
            
        self.selected_streams[sensor] = set(valid_streams)
        logging.info(f"Selected {len(valid_streams)} streams for sensor {sensor}")
        
    def select_all_streams(self) -> None:
        """Select all available streams for all sensors."""
        for sensor in self.sensor_list:
            self.selected_streams[sensor] = set(self.available_streams)
            
    def is_stream_selected(self, sensor: str, stream: str) -> bool:
        """Check if a specific stream is selected for loading."""
        if sensor not in self.selected_streams:
            return False
        return stream in self.selected_streams[sensor]
        
    def get_selected_streams(self, sensor: str) -> List[str]:
        """Get all selected streams for a specific sensor."""
        if sensor not in self.selected_streams:
            return []
        return list(self.selected_streams[sensor])
        
    def get_all_selected_combinations(self) -> List[tuple]:
        """Get all selected sensor-stream combinations."""
        combinations = []
        for sensor, streams in self.selected_streams.items():
            for stream in streams:
                combinations.append((sensor, stream))
        return combinations