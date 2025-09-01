import numpy as np

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
        - tuple: (match_percentage, mismatch_percentage) rounded to 5 decimal places
        """
        total = self.match_count + self.mismatch_count
        if total == 0:
            return 0.0, 0.0
        
        match_per = round((self.match_count / total) * 100, 5)
        mismatch_per = round((self.mismatch_count / total) * 100, 5)
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
        data_records = {}
        data_records['I'] = []  # Input only values (not matching output)
        data_records['O'] = []  # Output only values (not matching input)
        data_records['M'] = []  # Matching values (both in input and output)
        
        # Initialize counters
        match_count = 0
        mismatch_count = 0
        
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
            
            # Process each scan index
            for scan_idx in scan_indexs:
                if (input_data and scan_idx not in input_data) or (output_data and scan_idx not in output_data):
                    continue
                
                input_data_values = input_data.get(scan_idx) if input_data else None
                output_data_values = output_data.get(scan_idx) if output_data else None
                data_in_values = np.array([])
                data_out_values = np.array([])

                # Get input data if available
                if input_data_values and grp_idx_in is not None and plt_idx_in is not None:
                    if grp_idx_in < len(input_data_values) and plt_idx_in < len(input_data_values[grp_idx_in]):
                        data_in_values = input_data_values[grp_idx_in][plt_idx_in]
                        
                        # Ensure data_in_values is iterable
                        if isinstance(data_in_values, (np.uint32, np.uint8, np.float32, int, float)):
                            data_in_values = [data_in_values]
                        if not isinstance(data_in_values, (list, np.ndarray)):
                            data_in_values = []

                # Get output data if available
                if output_data_values and grp_idx_out is not None and plt_idx_out is not None:
                    if grp_idx_out < len(output_data_values) and plt_idx_out < len(output_data_values[grp_idx_out]):
                        data_out_values = output_data_values[grp_idx_out][plt_idx_out]
                        
                        # Ensure data_out_values is iterable
                        if isinstance(data_out_values, (np.uint32, np.uint8, np.float32, int, float)):
                            data_out_values = [data_out_values]
                        if not isinstance(data_out_values, (list, np.ndarray)):
                            data_out_values = []

                # Process and categorize values
                if len(data_in_values) > 0 and len(data_out_values) > 0:
                    # Find matching and non-matching values
                    for val_in in data_in_values:
                        # Compare values with 2 decimal precision
                        val_in_rounded = round(float(val_in), 2)
                        matches = [round(float(val_out), 2) for val_out in data_out_values]
                        
                        if val_in_rounded in matches:
                            # Matching value
                            data_records['M'].append([scan_idx, val_in])
                            match_count += 1
                        else:
                            # Input-only value
                            data_records['I'].append([scan_idx, val_in])
                            mismatch_count += 1
                    
                    # Find output-only values (not in input)
                    for val_out in data_out_values:
                        val_out_rounded = round(float(val_out), 2)
                        input_values = [round(float(val_in), 2) for val_in in data_in_values]
                        
                        if val_out_rounded not in input_values:
                            data_records['O'].append([scan_idx, val_out])
                            mismatch_count += 1
                
                # If we only have input values
                elif len(data_in_values) > 0:
                    for val_in in data_in_values:
                        data_records['I'].append([scan_idx, val_in])
                        mismatch_count += 1
                
                # If we only have output values
                elif len(data_out_values) > 0:
                    for val_out in data_out_values:
                        data_records['O'].append([scan_idx, val_out])
                        mismatch_count += 1

        except (ValueError, AttributeError) as e:
            print(f"Error processing data for signal {signal_name}: {str(e)}")
            return {}
            
        # Add match/mismatch info to data_records
        data_records['match_count'] = match_count
        data_records['mismatch_count'] = mismatch_count
        
        return data_records