from typing import Dict, List, Optional, Any
import itertools
from collections import defaultdict

class DataModelStorage:
    """
    A storage class for managing signal data with hierarchical relationships.
    Implements a bidirectional mapping between values and signals with support for parent-child relationships.
    """
    
    def __init__(self):
        # Bidirectional mapping between values and signals
        self._value_to_signal: Dict[str, str] = {}
        self._signal_to_value: Dict[str, Any] = {}
        
        # Main data container for storing scan index data
        self._data_container: Dict[str, List] = {}
        
        # Counter for unique identifiers
        self._parent_counter: int = -1
        self._child_counter: int = -1
        
    def initialize(self, scan_index: List[int]) -> None:
        """
        Initialize the data container with scan indices.
        
        Args:
            scan_index: List of scan indices to initialize the container with
        """
        # Use defaultdict to avoid checking if key exists later
        self._data_container = {str(j): [] for j in scan_index}
        
    def init_parent(self, stream_name) -> None:
        """Reset child counter when starting a new parent group."""
        self._parent_counter += 1
        self._child_counter = -1
        self.stream_name = stream_name

    def set_value(self, dataset: Any, scan_index: str, signal_name: str, grp_name: str) -> str:
        """
        Set a value in the storage with group relationship.
        
        Args:
            dataset: The data to store
            scan_index: The scan index to store the data under
            signal_name: Name of the signal
            grp_name: Name of the group this signal belongs to
            
        Returns:
            str: The generated key for the stored data
        """
        # Check if this is a new parent group
        is_new_parent = grp_name not in self._signal_to_value and self._child_counter == -1
        
        if is_new_parent:
            # Handle new parent group
            key_grp = f"{self._parent_counter}_None"
            self._child_counter += 1
            key_stream = f"{self._parent_counter}_{self._child_counter}"
            
            # Process and store the data
            self._process_dataset(dataset, key_stream, signal_name, key_grp)
        else:
            # Handle child item
            self._child_counter += 1
            key = f"{self._parent_counter}_{self._child_counter}"
            
            # Process and store data for child
            for row, scanidx in itertools.zip_longest(dataset, self._data_container, fillvalue=[]):
                if row is None:
                    print(f"Missing data in {signal_name} in {grp_name} in scanindex = {scanidx}")
                self._data_container[scanidx][-1].append(row)
                
            # Update mappings
            self._value_to_signal[key] = signal_name
            
            # Update signal-to-value mapping with optimized approach
            if signal_name not in self._signal_to_value:
                self._signal_to_value[signal_name] = [{grp_name: key}]
            else:
                signal_value = self._signal_to_value[signal_name]
                if isinstance(signal_value, list):
                    signal_value.append({grp_name: key})
                else:
                    self._signal_to_value[signal_name] = [{grp_name: key}]
                    
        # Return key for later reference
        return key if not is_new_parent else key_stream
        
    def _process_dataset(self, dataset, key_stream, signal_name, key_grp):
        """Helper method to process and store dataset for new parent groups."""
        # Process all rows in the dataset and store them
        for row, scanidx in itertools.zip_longest(dataset, self._data_container, fillvalue=[]):
            if row is None:
                print(f"Missing data in {signal_name} in scanindex = {scanidx}")
            self._data_container[scanidx].append([row])
            
        # Update mappings
        self._value_to_signal[key_stream] = signal_name
        self._value_to_signal[key_grp] = self.stream_name
        self._signal_to_value[signal_name] = key_stream
        self._signal_to_value[self.stream_name] = key_grp
    
    def get_sig_val_mapper(self, signal_name: str, grp_name: Optional[str] = None) -> Optional[Any]:
        """
        Get the value mapping for a signal, optionally filtered by group name.
        
        Args:
            signal_name: Name of the signal to look up
            grp_name: Optional group name to filter the results
            
        Returns:
            Optional[Any]: The value mapping if found, None otherwise
        """
        if signal_name not in self._signal_to_value:
            return None
            
        value_mapping = self._signal_to_value[signal_name]
        
        # If no group name specified, return the entire mapping
        if grp_name is None:
            return value_mapping
            
        # Handle nested dictionary structure
        if isinstance(value_mapping, dict):
            return value_mapping.get(grp_name)
            
        # Handle list of dictionaries structure (legacy support)
        if isinstance(value_mapping, list):
            for group_dict in value_mapping:
                if grp_name in group_dict:
                    return group_dict[grp_name]
            return None
            
        # Handle direct mapping structure
        return value_mapping
    
    def get_value_from_data_container(self, signal_name: str) -> Optional[Any]:
        """
        Get data from the container for a given scan index and signal name.
        
        Args:
            signal_name: Name of the signal
            
        Returns:
            Optional[Any]: The stored data if found, None otherwise
        """
        scan_index = 0
        if signal_name not in self._signal_to_value:
            return None
            
        value_mapping = self._signal_to_value[signal_name]
        if isinstance(value_mapping, list):
            # Return all values for this signal across different groups
            result = []
            for mapping in value_mapping:
                for key in mapping.values():
                    parent_idx, child_idx = key.split('_')
                    parent_idx = int(parent_idx)
                    if child_idx == 'None':
                        result.extend(self._data_container[scan_index][parent_idx - 1][0])
                    else:
                        result.extend(self._data_container[scan_index][parent_idx - 1][int(child_idx)])
            return result
        else:
            # Single value case
            parent_idx, child_idx = value_mapping.split('_')
            parent_idx = int(parent_idx)
            if child_idx == 'None':
                return self._data_container[scan_index][parent_idx - 1][0]
            return self._data_container[scan_index][parent_idx - 1][int(child_idx)]
    
    def get_data_container(self) -> Optional[Any]:
        return self._data_container
    
    def get_val_sig_mapper(self) -> Optional[Any]:
        return self._value_to_signal
    
    def set_val_by_pos(self, position: int, data, signal_name: str) -> str:
        """
        Set a value at a specific position.
        
        Args:
            position: The position to store the data
            data: The data to store
            signal_name: Name of the signal
            
        Returns:
            str: The generated key for the stored data
        """
        key = f"pos_{position}"
        self._data_container[key] = data
        self._value_to_signal[key] = signal_name
        self._signal_to_value[signal_name] = key
        return key
    
    def get_value(self, signal_name: str) -> Optional[Any]:
        """
        Get the value key for a given signal name.
        
        Args:
            signal_name: Name of the signal
            
        Returns:
            Optional[Any]: The value mapping if found, None otherwise
        """
        return self._signal_to_value.get(signal_name)
    
    def clear(self) -> None:
        """Clear all stored data and reset counters."""
        self._value_to_signal.clear()
        self._signal_to_value.clear()
        self._data_container.clear()
        self._parent_counter = 0
        self._child_counter = -1