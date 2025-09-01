from typing import Dict, List, Optional, Any
import itertools
from collections import defaultdict
import logging
import numpy as np

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
                logging.debug(f"Missing scan indices at this signal {sensor}/{stream}: {missing_indices}")
        
        # Use defaultdict to avoid checking if key exists later
        self._data_container = {j: [] for j in scan_index}


    def init_parent(self, stream_name) -> None:
        """Reset child counter when starting a new parent group."""
        self._parent_counter += 1
        self._child_counter = -1
        self.stream_name = stream_name

    def set_value(self, dataset: Any, signal_name: str, grp_name: str) -> str:
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
    
    def clear(self) -> None:
        """Clear all stored data and reset counters."""
        self._value_to_signal.clear()
        self._signal_to_value.clear()
        self._data_container.clear()
        self._parent_counter = 0
        self._child_counter = -1
        
    @staticmethod
    def get_data( input_data, output_data ,signal_name, scan_indexs , grp_name=None):#need to add grp_name funtionality
        """
        Get data records for the specified signal from input and output data.
        
        This static method can be called from anywhere by providing all required parameters.
        
        Args:
            signal_name: Name of the signal to get data for
            scan_indexs: List of unique scan indices
            input_data: Container for input data
            output_data: Container for output data
            signal_to_value_map_in: Mapping of signal names to input values
            signal_to_value_map_out: Mapping of signal names to output values
            
        Returns:
            Dictionary with keys containing lists of [scan_idx, value] pairs:
              - 'I': All input values
              - 'O': All output values
              - 'M': Values that match between input and output

        """
        # Improved memory structure to reduce redundant lists
        data_records = {
            'I': [],  # All input values
            'O': [],  # All output values
        }
        data_dict = {}

        # Check if signal exists in input and output maps
        unique_in = input_data._signal_to_value.get(signal_name) if input_data._signal_to_value else None
        unique_out = output_data._signal_to_value.get(signal_name) if output_data._signal_to_value else None
        
        # Return early if signal not found in either map
        if not unique_in and not unique_out:
            return "no_data_in_hdf", {}
            
        # try:
        # Parse group and plot indices for input and output
        if isinstance(unique_in, str) and isinstance(unique_out, str):
            grp_idx_in, plt_idx_in = map(int, unique_in.split('_')) if unique_in else (None, None)
            grp_idx_out, plt_idx_out = map(int, unique_out.split('_')) if unique_out else (None, None)
        elif  isinstance(unique_in, list) and isinstance(unique_out, list):
            grp_idx_in, plt_idx_in = map(int, list(unique_in[0].values())[0].split('_')) if unique_in else (None, None)
            grp_idx_out, plt_idx_out = map(int,list(unique_out[0].values())[0].split('_')) if unique_out else (None, None)

        # Pre-filter valid scan indices to avoid repeated lookups
        valid_scan_indices = [
            idx for idx in scan_indexs 
            if (input_data._data_container.keys() and idx in input_data._data_container.keys()) or (output_data._data_container.keys() and idx in output_data._data_container.keys())
        ]
        
        # Process all scan indices at once
        all_in_values = []
        all_out_values = []
        
        # First pass: collect all values in a single loop
        for scan_idx in valid_scan_indices:

            # Get input data if available
            if input_data._data_container[scan_idx] and grp_idx_in is not None and plt_idx_in is not None:
                if grp_idx_in < len(input_data._data_container[scan_idx]) and plt_idx_in < len(input_data._data_container[scan_idx][grp_idx_in]):
                    data_in = input_data._data_container[scan_idx][grp_idx_in][plt_idx_in]
                    data_dict[scan_idx] = input_data._data_container[scan_idx][grp_idx_in]
                    # Convert to list if scalar value
                    if isinstance(data_in, (np.uint32, np.uint8, np.float32, int, float)):
                        data_in = [data_in]
                    elif not isinstance(data_in, (list, np.ndarray)):
                        data_in = [data_in]
                        
                    for val in data_in:
                        all_in_values.append((scan_idx, round(float(val), 2)))

            # Get output data if available
            if output_data._data_container[scan_idx] and grp_idx_out is not None and plt_idx_out is not None:
                if grp_idx_out < len(output_data._data_container[scan_idx]) and plt_idx_out < len(output_data._data_container[scan_idx][grp_idx_out]):
                    data_out = output_data._data_container[scan_idx][grp_idx_out][plt_idx_out]
                    data_dict[scan_idx].append(output_data._data_container[scan_idx][grp_idx_out])
                    # Convert to list if scalar value
                    if isinstance(data_out, (np.uint32, np.uint8, np.float32, int, float)):
                        data_out = [data_out]
                    elif not isinstance(data_out, (list, np.ndarray)):
                        data_out = [data_out]
                        
                    for val in data_out:
                        all_out_values.append((scan_idx, round(float(val), 2)))

        # Fill the data_records with all input values
        for scan_idx, val in all_in_values:
            data_records['I'].append([scan_idx, val])
        
        # Fill the data_records with all output values
        for scan_idx, val in all_out_values:
            data_records['O'].append([scan_idx, val])
        
        # except (ValueError, AttributeError) as e:
        # print(f"Error processing data for signal {signal_name}: {str(e)}")
        # return {}, {}
            
        return data_records, data_dict
