from typing import Dict, List, Optional, Any, Union
import itertools
from collections import defaultdict
import logging
import gc
import numpy as np
from collections import OrderedDict

class DataModelStorage:
    """
    A storage class for managing signal data with hierarchical relationships.
    Implements a bidirectional mapping between values and signals with support for parent-child relationships.
    Memory-optimized version to handle large datasets efficiently.
    """
    
    def __init__(self):
        # Bidirectional mapping between values and signals
        self._value_to_signal: Dict[str, str] = {}
        self._signal_to_value: Dict[str, Any] = {}
        
        # Main data container for storing scan index data
        self._data_container: Dict[int, List] = {}
        
        # Counter for unique identifiers
        self._parent_counter: int = -1
        self._child_counter: int = -1
        
        # Memory management flag
        self._memory_cleaned: bool = False
        
    def initialize(self, scan_index, sensor, stream) -> None:
        """
        Initialize the data container with scan indices.
        
        Args:
            scan_index: List or NumPy array of scan indices to initialize the container with
            sensor: Name of the sensor
            stream: Name of the data stream
        """
        # Reset the memory cleaned flag for new data
        self._memory_cleaned = False
        
        # Handle empty scan index
        if not scan_index or len(scan_index) == 0:
            self._data_container = {}
            return
            
        # Optimize: Use a direct copy instead of comprehension for large sets
        if isinstance(scan_index, np.ndarray):
            scan_index = scan_index.tolist()
            
        # Check if scan_index is sorted and sequential (optional check for debug purposes)
        if len(scan_index) > 0:
            # Minimize memory: don't create the full expected_scan_index list for large arrays
            min_idx = min(scan_index)
            max_idx = max(scan_index)
            
            # Only log if the number of missing indices is small relative to total size
            if (max_idx - min_idx + 1) - len(scan_index) < 100:  # Only check if reasonable
                expected_scan_index = set(range(min_idx, max_idx + 1))
                missing_indices = expected_scan_index.difference(set(scan_index))
                
                if missing_indices and len(missing_indices) < 50:  # Only log a reasonable number
                    logging.debug(f"Missing scan indices at this signal {sensor}/{stream}: {sorted(list(missing_indices))}")
        
        # Initialize container with empty lists - more efficient than dict comprehension for large datasets
        self._data_container = {}
        for idx in scan_index:
            self._data_container[idx] = []


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

    def set_value(self, dataset: Any, scan_index: List[int], signal_name: str, grp_name: str) -> str:
        """
        Set a value in the storage with group relationship.
        Memory-optimized version that processes data in chunks and releases memory as soon as possible.
        
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
            
            # Process and store the data more efficiently
            self._process_dataset(dataset, key_stream, signal_name, key_grp)
            return key_stream
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
            
            # Process data in chunks for better memory efficiency
            # Use iterators instead of creating a full zip object
            scan_keys = list(self._data_container.keys())
            
            # Process data in batches to reduce memory pressure
            BATCH_SIZE = 1000  # Adjust based on typical dataset size
            for i in range(0, dataset_len, BATCH_SIZE):
                end_idx = min(i + BATCH_SIZE, dataset_len)
                for j in range(i, end_idx):
                    if j < len(scan_keys):
                        scanidx = scan_keys[j]
                        if dataset[j] is not None:
                            # Append directly to the existing list instead of creating a new one
                            if self._data_container[scanidx] and self._data_container[scanidx][-1] is not None:
                                self._data_container[scanidx][-1].append(dataset[j])
                
            # Update mappings efficiently
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
            
            return key
        
    def _process_dataset(self, dataset, key_stream, signal_name, key_grp):
        """Helper method to process and store dataset for new parent groups.
        Memory-optimized version with batch processing."""
        
        # Get the length of dataset and data_container
        dataset_len = len(dataset) if dataset is not None else 0
        container_len = len(self._data_container)
        
        # Skip if lengths don't match
        if dataset_len != container_len:
            print(f"Skipping plot for {signal_name}: dataset length ({dataset_len}) does not match scan indices length ({container_len})")
            return
        
        # Process data in batches to reduce memory pressure
        scan_keys = list(self._data_container.keys())
        BATCH_SIZE = 1000  # Adjust based on typical dataset size
        
        for i in range(0, dataset_len, BATCH_SIZE):
            end_idx = min(i + BATCH_SIZE, dataset_len)
            for j in range(i, end_idx):
                if j < len(scan_keys):
                    scanidx = scan_keys[j]
                    if dataset[j] is not None:
                        # Store each row directly, avoiding intermediate list creations
                        self._data_container[scanidx].append([dataset[j]])
            
            # Force memory release after each batch
            if i + BATCH_SIZE < dataset_len:
                gc.collect()
            
        # Update mappings
        self._value_to_signal[key_stream] = signal_name
        self._value_to_signal[key_grp] = self.stream_name
        self._signal_to_value[signal_name] = key_stream
        self._signal_to_value[self.stream_name] = key_grp
    

    @staticmethod
    def get_data(signal_name: str, grp_name: str, input_data,output_data):
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
              - 'MI': Mismatch input (input-only values)
              - 'MO': Mismatch output (output-only values)
        """
        # Improved memory structure to reduce redundant lists
        data_records = {
            'I': [],  # All input values
            'O': [],  # All output values
            'M': [],  # Matching values (both in input and output)
            'MI': [], # Mismatch input (input-only values)
            'MO': [], # Mismatch output (output-only values)
            'match_count': 0,
            'mismatch_count': 0
        }
        
        # Check if signal exists in input and output maps
        unique_in = input_data._signal_to_value.get(signal_name) if input_data._signal_to_value else None
        unique_out = output_data._signal_to_value.get(signal_name) if output_data._signal_to_value else None
        
        # Return early if signal not found in either map
        if not unique_in and not unique_out:
            return "no_data_in_hdf", {}
            
        try:
            # Parse group and plot indices for input and output
            grp_idx_in, plt_idx_in = map(int, list(unique_in[0].values())[0].split('_')) if unique_in else (None, None)
            grp_idx_out, plt_idx_out = map(int, list(unique_out[0].values())[0].split('_')) if unique_out else (None, None)
                    # Get all unique keys from both data containers (deduplication)
            
            unique_keys = list(OrderedDict.fromkeys(list(input_data._data_container.keys()) + list(output_data._data_container.keys())))
            scan_indexs = tuple(unique_keys)  # Convert to tuple for caching
        
            # Pre-filter valid scan indices to avoid repeated lookups
            valid_scan_indices = [
                idx for idx in scan_indexs 
                if (input_data and idx in input_data) or (output_data and idx in output_data)
            ]
            
            # Process all scan indices at once - better memory efficiency
            all_in_values = []
            all_out_values = []
            
            # First pass: collect all values in a single loop
            for scan_idx in valid_scan_indices:
                input_data_values = input_data.get(scan_idx) if input_data else None
                output_data_values = output_data.get(scan_idx) if output_data else None
                
                # Get input data if available
                if input_data_values and grp_idx_in is not None and plt_idx_in is not None:
                    if grp_idx_in < len(input_data_values) and plt_idx_in < len(input_data_values[grp_idx_in]):
                        data_in = input_data_values[grp_idx_in][plt_idx_in]
                        
                        # Convert to list if scalar value
                        if isinstance(data_in, (np.uint32, np.uint8, np.float32, int, float)):
                            data_in = [data_in]
                        elif not isinstance(data_in, (list, np.ndarray)):
                            data_in = [data_in]
                            
                        for val in data_in:
                            all_in_values.append((scan_idx, float(val)))

                # Get output data if available
                if output_data_values and grp_idx_out is not None and plt_idx_out is not None:
                    if grp_idx_out < len(output_data_values) and plt_idx_out < len(output_data_values[grp_idx_out]):
                        data_out = output_data_values[grp_idx_out][plt_idx_out]
                        
                        # Convert to list if scalar value
                        if isinstance(data_out, (np.uint32, np.uint8, np.float32, int, float)):
                            data_out = [data_out]
                        elif not isinstance(data_out, (list, np.ndarray)):
                            data_out = [data_out]
                            
                        for val in data_out:
                            all_out_values.append((scan_idx, float(val)))
            
            # Create dictionaries for fast lookup to find matching values
            in_dict = defaultdict(list)
            for scan_idx, val in all_in_values:
                # Round to 2 decimal points for comparison
                in_dict[round(val, 2)].append((scan_idx, val))
                
            out_dict = defaultdict(list)
            for scan_idx, val in all_out_values:
                # Round to 2 decimal points for comparison
                out_dict[round(val, 2)].append((scan_idx, val))
            
            # Fill the data_records with all input values
            for scan_idx, val in all_in_values:
                data_records['I'].append([scan_idx, val])
            
            # Fill the data_records with all output values
            for scan_idx, val in all_out_values:
                data_records['O'].append([scan_idx, val])
            
            # Process matching and mismatching values
            # Track which values have been matched
            matched_in_values = set()
            matched_out_values = set()
            
            # Find matching values for 'M'
            for rounded_val in in_dict.keys():
                if rounded_val in out_dict:
                    # There's at least one match for this value
                    for scan_idx, original_val in in_dict[rounded_val]:
                        data_records['M'].append([scan_idx, original_val])
                        matched_in_values.add((scan_idx, rounded_val))
                        data_records['match_count'] += 1
                    
                    # Mark corresponding output values as matched
                    for scan_idx, original_val in out_dict[rounded_val]:
                        matched_out_values.add((scan_idx, rounded_val))
            
            # Find input-only values for 'MI'
            for rounded_val, entries in in_dict.items():
                for scan_idx, original_val in entries:
                    if (scan_idx, rounded_val) not in matched_in_values:
                        data_records['MI'].append([scan_idx, original_val])
            
            # Find output-only values for 'MO'
            for rounded_val, entries in out_dict.items():
                for scan_idx, original_val in entries:
                    if (scan_idx, rounded_val) not in matched_out_values:
                        data_records['MO'].append([scan_idx, original_val])
            
            # Calculate the mismatch count
            data_records['mismatch_count'] = len(data_records['MI']) + len(data_records['MO'])

        except (ValueError, AttributeError) as e:
            print(f"Error processing data for signal {signal_name}: {str(e)}")
            return {}, {}
            
        data_dict = {}
        return data_records, data_dict