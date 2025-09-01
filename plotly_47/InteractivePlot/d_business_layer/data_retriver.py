import numpy as np
import pandas as pd
from collections import defaultdict
from functools import lru_cache

class DataRetriever:
    """
    Class responsible for retrieving and processing data from input and output containers.
    """
    
    def __init__(self):
        # Initialize counters
        self.match_count = 0
        self.mismatch_count = 0

    def get_match_mismatch_per(self):
        """
        Calculate and return match and mismatch percentages
        
        Returns:
        - tuple: (match_percentage, mismatch_percentage) rounded to 2 decimal places
        """
        total = self.match_count + self.mismatch_count
        if total == 0:
            return 0.0, 0.0
        
        match_per = round((self.match_count / total) * 100, 2)
        mismatch_per = round((self.mismatch_count / total) * 100, 2)
        return match_per, mismatch_per

    @staticmethod
    @lru_cache(maxsize=256)
    def _get_value_indices(unique_map, signal_name):
        """
        Extract group and plot indices from signal map with caching.
        
        Args:
            unique_map: Signal to value map
            signal_name: Name of the signal
            
        Returns:
            tuple: (group_index, plot_index) or (None, None) if not found
        """
        if not unique_map or signal_name not in unique_map:
            return None, None
            
        try:
            indices_str = list(unique_map[signal_name][0].values())[0]
            return map(int, indices_str.split('_'))
        except (IndexError, ValueError, KeyError):
            return None, None

    @staticmethod
    def _extract_data(data_values, grp_idx, plt_idx):
        """
        Extract data values from the data container.
        
        Args:
            data_values: Data values from the container
            grp_idx: Group index
            plt_idx: Plot index
            
        Returns:
            list: List of values or empty list if indices are invalid
        """
        if not data_values or grp_idx is None or plt_idx is None:
            return []
            
        if grp_idx >= len(data_values) or plt_idx >= len(data_values[grp_idx]):
            return []
            
        data = data_values[grp_idx][plt_idx]
        
        # Convert to list if scalar value
        if isinstance(data, (np.uint32, np.uint8, np.float32, int, float)):
            return [float(data)]
        elif isinstance(data, (list, np.ndarray)):
            return [float(val) for val in data]
        
        return []

    @staticmethod
    def get_data(signal_name, scan_indexs, input_data, output_data, signal_to_value_map_in, signal_to_value_map_out):
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
            Dictionary with 'I', 'O', 'M' keys each containing lists of [scan_idx, value] pairs
        """
        data_records = {
            'I': [],  # Input only values (not matching output)
            'O': [],  # Output only values (not matching input)
            'M': [],  # Matching values (both in input and output)
            'match_count': 0,
            'mismatch_count': 0
        }
        
        # Extract indices only once
        grp_idx_in, plt_idx_in = DataRetriever._get_value_indices(signal_to_value_map_in, signal_name)
        grp_idx_out, plt_idx_out = DataRetriever._get_value_indices(signal_to_value_map_out, signal_name)
        
        # Return early if signal not found in either map
        if (grp_idx_in is None and grp_idx_out is None):
            return "no_data_in_hdf"
            
        try:
            # Pre-filter valid scan indices to avoid repeated lookups
            valid_scan_indices = []
            for idx in scan_indexs:
                if (input_data and idx in input_data) or (output_data and idx in output_data):
                    valid_scan_indices.append(idx)
            
            # Use NumPy arrays for better performance with large datasets
            all_in_values = []
            all_out_values = []
            
            # First pass: collect all values in a single loop - more memory efficient
            for scan_idx in valid_scan_indices:
                input_data_values = input_data.get(scan_idx) if input_data else None
                output_data_values = output_data.get(scan_idx) if output_data else None
                
                # Process input data
                in_values = DataRetriever._extract_data(input_data_values, grp_idx_in, plt_idx_in)
                for val in in_values:
                    all_in_values.append((scan_idx, val))
                
                # Process output data
                out_values = DataRetriever._extract_data(output_data_values, grp_idx_out, plt_idx_out)
                for val in out_values:
                    all_out_values.append((scan_idx, val))
            
            # Fast matching algorithm with lower memory footprint
            in_dict = defaultdict(list)
            for scan_idx, val in all_in_values:
                # Round to 4 decimal points for comparison to reduce floating point errors
                # but keep more precision than before
                in_dict[round(val, 4)].append((scan_idx, val))
                
            out_dict = defaultdict(list)
            for scan_idx, val in all_out_values:
                # Round to 4 decimal points for comparison
                out_dict[round(val, 4)].append((scan_idx, val))
            
            # Find matches and mismatches efficiently
            matched_vals = set(in_dict.keys()) & set(out_dict.keys())
            in_only_vals = set(in_dict.keys()) - matched_vals
            out_only_vals = set(out_dict.keys()) - matched_vals
            
            # Process matches
            for val in matched_vals:
                for scan_idx, original_val in in_dict[val]:
                    data_records['M'].append([scan_idx, original_val])
                    data_records['match_count'] += 1
            
            # Process input-only values
            for val in in_only_vals:
                for scan_idx, original_val in in_dict[val]:
                    data_records['I'].append([scan_idx, original_val])
                    data_records['mismatch_count'] += 1
            
            # Process output-only values
            for val in out_only_vals:
                for scan_idx, original_val in out_dict[val]:
                    data_records['O'].append([scan_idx, original_val])
                    data_records['mismatch_count'] += 1

        except Exception as e:
            print(f"Error processing data for signal {signal_name}: {str(e)}")
            return {}
            
        return data_records