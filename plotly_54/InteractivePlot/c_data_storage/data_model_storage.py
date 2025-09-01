from typing import Dict, List, Optional, Any
import itertools
from collections import defaultdict
import logging

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
        
    def initialize(self, scan_index, sensor, stream) -> None:
        """
        Initialize the data container with scan indices.
        
        Args:
            scan_index: List or NumPy array of scan indices to initialize the container with
        """
        
        # Check if scan_index is sorted and sequential
        if len(scan_index) > 0:
            scan_index.sort()  # Ensure the list is sorted
            expected_scan_index = list(range(min(scan_index), max(scan_index) + 1))
            
            # Find missing indices
            missing_indices = [i for i in expected_scan_index if i not in scan_index]
            
            if missing_indices:
                logging.info(f"Missing scan indices at this signal {sensor}/{stream}: {missing_indices}")
        
        # Use defaultdict to avoid checking if key exists later
        self._data_container = {j: [] for j in scan_index}


        #for future if we need to find duplicate
        # # Handle empty scan index list/array
        # if isinstance(scan_index, np.ndarray):
        #     if scan_index.size == 0:
        #         self._data_container = {}
        #         self.miss_dup_idx = []  # Initialize empty miss_dup_idx for other methods
        #         return
        # elif not scan_index:
        #     self._data_container = {}
        #     self.miss_dup_idx = []  # Initialize empty miss_dup_idx for other methods
        #     return
        
        # # Get the first index and the expected range
        # first_idx = scan_index[0]
        # lowest_idx = np.min(scan_index) if isinstance(scan_index, np.ndarray) else min(scan_index)
        
        # # Check if the lowest index matches the first index
        # if lowest_idx != first_idx:
        #     logging.warning(f"First scan index {first_idx} is not the lowest index {lowest_idx} for {sensor}/{stream}. Skipping processing.")
        #     self._data_container = {}
        #     self.miss_dup_idx = list(range(len(scan_index)))  # Mark all indices as problematic
        #     return
        
        # # Initialize containers for missing and duplicate indices
        # missing_indices = []
        # duplicate_indices = []
        # processed_indices = set()
        
        # # Create a new data container
        # self._data_container = {}
        
        # # Get unique indices and expected maximum index
        # if isinstance(scan_index, np.ndarray):
        #     unique_indices = set(scan_index.tolist())
        # else:
        #     unique_indices = set(scan_index)
        # expected_max_idx = first_idx + len(unique_indices) - 1
        
        # # List to track indices to skip (duplicates or out of range)
        # self.miss_dup_idx = []
        
        # # Process all indices to identify missing and duplicate values
        # for idx, scan_idx in enumerate(scan_index):
        #     # Check if this index is too large to process
        #     if scan_idx > expected_max_idx:
        #         array_length = scan_index.size if isinstance(scan_index, np.ndarray) else len(scan_index)
        #         missing_indices.extend(range(idx, array_length))
        #         self.miss_dup_idx.extend(range(idx, array_length))
        #         break
                
        #     # Check for duplicates
        #     if scan_idx in processed_indices:
        #         duplicate_indices.append(idx)
        #         self.miss_dup_idx.append(idx)
        #         continue
                
        #     # Process valid index
        #     self._data_container[scan_idx] = []
        #     processed_indices.add(scan_idx)
        
        # # Identify missing indices in the sequence
        # expected_sequence = set(range(first_idx, expected_max_idx + 1))
        # missing_values = expected_sequence - processed_indices
        
        # # Log missing and duplicate information
        # if missing_values:
        #     logging.debug(f"Missing scan index values at {sensor}/{stream}: {sorted(missing_values)}")
            
        # if duplicate_indices:
        #     logging.debug(f"Duplicate scan indices at positions {sensor}/{stream}: {duplicate_indices}")
            
        # if missing_indices:
        #     logging.debug(f"Skipped scan indices at positions {sensor}/{stream}: {missing_indices}")
        

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
            
            # Get the length of dataset and data_container
            dataset_len = len(dataset) if dataset is not None else 0
            container_len = len(self._data_container)
            
            # Skip if lengths don't match
            if dataset_len != container_len:
                # Route messages about skipping child processing to logs file
                logging.debug(f"Skipping child processing for {signal_name} in {grp_name}: dataset length ({dataset_len}) does not match scan indices length ({container_len})")
                return key
            
            # Process and store data for child
            for idx, (row, scanidx) in enumerate(zip(dataset, self._data_container)):
                # if idx in self.miss_dup_idx:
                #     pass
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
        
        # Get the length of dataset and data_container
        dataset_len = len(dataset) if dataset is not None else 0
        container_len = len(self._data_container)
        
        # Skip if lengths don't match
        if dataset_len != container_len:
            print(f"Skipping plot for {signal_name}: dataset length ({dataset_len}) does not match scan indices length ({container_len})")
            return
        
        # Process all rows in the dataset and store them
        for idx, (row, scanidx) in enumerate(zip(dataset, self._data_container)):
            # if idx in self.miss_dup_idx:
            #         pass
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
            
        # Handle list of dictionaries structure
        if isinstance(value_mapping, list):
            for group_dict in value_mapping:
                if grp_name in group_dict:
                    return group_dict[grp_name]
            return None
            
        # Handle direct mapping structure
        return value_mapping
    
    def get_value_from_data_container(self, signal_name: str, grp_name: Optional[str] = None) -> Optional[Any]:
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
        return data_record
    
    def clear(self) -> None:
        """Clear all stored data and reset counters."""
        self._value_to_signal.clear()
        self._signal_to_value.clear()
        self._data_container.clear()
        self._parent_counter = 0
        self._child_counter = -1