import numpy as np

class DataRetriever:
    """
    Class responsible for retrieving and processing data from input and output containers.
    """
    
    @staticmethod
    def get_data(signal_name, unique_keys, input_data, output_data, signal_to_value_map_in, signal_to_value_map_out):
        """
        Get data records for the specified signal from input and output data.
        
        This static method can be called from anywhere by providing all required parameters.
        
        Args:
            signal_name: Name of the signal to get data for
            unique_keys: List of unique scan indices
            input_data: Container for input data
            output_data: Container for output data
            signal_to_value_map_in: Mapping of signal names to input values
            signal_to_value_map_out: Mapping of signal names to output values
            
        Returns:
            List of tuples (scan_idx, value, type) for plotting
        """
        data_records_in = {}
        data_records_out = {}
        
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
            for scan_idx in unique_keys:
                if (input_data and scan_idx not in input_data) or (output_data and scan_idx not in output_data):
                    continue
                    
                input_data_values = input_data.get(scan_idx) if input_data else None
                output_data_values = output_data.get(scan_idx) if output_data else None
                
                # Get input data if available
                if input_data_values and grp_idx_in is not None and plt_idx_in is not None:
                    if (grp_idx_in < len(input_data_values) and plt_idx_in < len(input_data_values[grp_idx_in])):
                        data_in = input_data_values[grp_idx_in][plt_idx_in]
                        data_records_in[scan_idx] = [data_in, 'input']
                
                # Get output data if available
                if output_data_values and grp_idx_out is not None and plt_idx_out is not None:
                    if (grp_idx_out < len(output_data_values) and plt_idx_out < len(output_data_values[grp_idx_out])):
                        data_out = output_data_values[grp_idx_out][plt_idx_out]
                        data_records_out[scan_idx] = [data_out, 'output']
                            
        except (ValueError, AttributeError) as e:
            print(f"Error processing data for signal {signal_name}: {str(e)}")
            return {}, {}
            
        return data_records_in, data_records_out
