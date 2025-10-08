import numpy as np
import pandas as pd
from collections import defaultdict

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
        # Improved memory structure to reduce redundant lists
        data_records = {
            'I': [],  # Input only values (not matching output)
            'O': [],  # Output only values (not matching input)
            'M': [],  # Matching values (both in input and output)
            'match_count': 0,
            'mismatch_count': 0
        }
        
        # Check if signal exists in input and output maps
        unique_in = signal_to_value_map_in.get(signal_name) if signal_to_value_map_in else None
        unique_out = signal_to_value_map_out.get(signal_name) if signal_to_value_map_out else None
        
        # Return early if signal not found in either map
        if not unique_in and not unique_out:
            return "no_data_in_hdf"
            
        try:
            # Parse group and plot indices for input and output
            grp_idx_in, plt_idx_in = map(int, list(unique_in[0].values())[0].split('_')) if unique_in else (None, None)
            grp_idx_out, plt_idx_out = map(int, list(unique_out[0].values())[0].split('_')) if unique_out else (None, None)
            
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
                            data_in = []
                            
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
                            data_out = []
                            
                        for val in data_out:
                            all_out_values.append((scan_idx, float(val)))
            
            # Improved matching algorithm with O(n) complexity using hashtable
            # Create dictionaries for fast lookup
            in_dict = defaultdict(list)
            for scan_idx, val in all_in_values:
                # Round to 2 decimal points for comparison
                in_dict[round(val, 2)].append((scan_idx, val))
                
            out_dict = defaultdict(list)
            for scan_idx, val in all_out_values:
                # Round to 2 decimal points for comparison
                out_dict[round(val, 2)].append((scan_idx, val))
            
            # Find matches and mismatches
            processed_out_values = set()
            
            # Find matching values and input-only values
            for rounded_val, in_entries in in_dict.items():
                if rounded_val in out_dict:
                    # There's at least one match
                    for scan_idx, original_val in in_entries:
                        data_records['M'].append([scan_idx, original_val])
                        data_records['match_count'] += 1
                else:
                    # Input-only values
                    for scan_idx, original_val in in_entries:
                        data_records['I'].append([scan_idx, original_val])
                        data_records['mismatch_count'] += 1
            
            # Find output-only values
            for rounded_val, out_entries in out_dict.items():
                if rounded_val not in in_dict:
                    # Output-only values
                    for scan_idx, original_val in out_entries:
                        data_records['O'].append([scan_idx, original_val])
                        data_records['mismatch_count'] += 1

        except (ValueError, AttributeError) as e:
            print(f"Error processing data for signal {signal_name}: {str(e)}")
            return {}
            
        return data_records