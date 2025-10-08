from typing import Dict, List, Optional, Any
import itertools

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
        
        self._data_container = {str(j): [] for j in scan_index}
        
    def init_parent(self,stream_name) -> None:
        """Reset child counter when starting a new parent group."""
        self._parent_counter += 1
        self._child_counter = -1
        self.stream_name = stream_name

    def set_value(self, dataset: Any, scan_index: str, signal_name: str, grp_name: str) -> str:
        """
        Set a value in the storage with group relationship.
        
        Args:
            data: The data to store
            scan_index: The scan index to store the data under
            signal_name: Name of the signal
            grp_name: Name of the group this signal belongs to
            
        Returns:
            str: The generated key for the stored data
        """
        if grp_name  not in self._signal_to_value and self._child_counter == -1:
            key_grp = f"{self._parent_counter}_None"
            self._child_counter += 1
            key_stream = f"{self._parent_counter}_{self._child_counter}"
            for row, scanidx in itertools.zip_longest(dataset, self._data_container, fillvalue=[]):
                    
                if row is None:
                    print(f"Missing data in {signal_name} in {grp_name} in scanindex = {scanidx}")
                    
                self._data_container[scanidx].append([row])

            self._value_to_signal[key_stream] = signal_name
            self._value_to_signal[key_grp] = self.stream_name
            self._signal_to_value[signal_name] = key_stream
            self._signal_to_value[self.stream_name] = key_grp
            
        else:
            self._child_counter += 1
            key = f"{self._parent_counter}_{self._child_counter}"
            for row, scanidx in itertools.zip_longest(dataset, self._data_container, fillvalue=[]):
                    if row is None:
                        print(f"Missing data in {signal_name} in {grp_name} in scanindex = {scanidx}")
                    self._data_container[scanidx][-1].append(row)
            self._value_to_signal[key] = signal_name
            if signal_name not in self._signal_to_value:
                self._signal_to_value[signal_name] = [{grp_name: key}]
            else:
                if isinstance(self._signal_to_value[signal_name], list):
                    self._signal_to_value[signal_name].append({grp_name: key})
                else:
                    self._signal_to_value[signal_name] = [{grp_name: key}]


    
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
    

    def get_value_from_data_container(self,signal_name: str) -> Optional[Any]:
        """
        Get data from the container for a given scan index and signal name.
        
        Args:
            scan_index: The scan index to retrieve data from
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
                    parent_idx = int(key.split('_')[0])
                    child_idx = key.split('_')[1]
                    if child_idx == 'None':
                        result.extend(self._data_container[scan_index][parent_idx - 1][0])
                    else:
                        result.extend(self._data_container[scan_index][parent_idx - 1][int(child_idx)])
            return result
        else:
            # Single value case
            parent_idx = int(value_mapping.split('_')[0])
            child_idx = value_mapping.split('_')[1]
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
