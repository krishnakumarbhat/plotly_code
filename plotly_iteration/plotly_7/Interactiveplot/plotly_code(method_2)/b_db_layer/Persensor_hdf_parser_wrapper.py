import h5py
import numpy as np
from b_db_layer.hdf_parser import HDF5Parser
# from c_business_layer.data_model import DataContainer

class PersensorHdfParserWrapper:
    def __init__(self, data_container , address_map):
        """Initialize the parser wrapper with an address map."""
        self.address_map = address_map
        self.data_container = data_container

    def parse(self):
        """Parse input and output HDF5 files based on the address map."""
        for input_file, output_file in self.address_map.items():
            self._process_file(input_file, is_output=False)
            self._process_file(output_file, is_output=True)

        # print("Final Data Container:", self.data_container)

    def _process_file(self, file_path, is_output):
        """Process a single HDF5 file and store its parsed data."""
        with h5py.File(file_path, 'r') as file:
            file_type = "output" if is_output else "input"
            print(f"Processing {file_type} file: {file_path}")
            
            parser = HDF5Parser(file)
            parsed_data, unique_map, reverse_unique_map ,parent_map ,lenud = parser.parse()
            # print("Parent Map:", parent_map)
            # Storeing the parsed data in the data container
            if is_output:
                self.data_container.add_output_data(parsed_data, unique_map, reverse_unique_map,parent_map,lenud)
            else:
                self.data_container.add_input_data(parsed_data, unique_map, reverse_unique_map,parent_map ,lenud)
            if isinstance(parsed_data, dict):
                print("Parsed Data is a Dictionary:")
                for key, value in parsed_data.items():
                    if isinstance(value, np.ndarray):
                        print(f"Key: {key}")
                        print(f" - Shape: {value.shape}")
                        print(f" - Size: {value.size}")
                    else:
                        print(f"Key: {key} is not a NumPy array.")
            else:
                if isinstance(parsed_data, np.ndarray):
                    print("Parsed Data is a NumPy Array:")
                    print(f" - Shape: {parsed_data.shape}")
                    print(f" - Size: {parsed_data.size}")
                else:
                    print("Parsed Data is neither a NumPy array nor a dictionary.")

            print(f"Parsed {file_type.capitalize()} Data:", parsed_data)
            print("Unique Map:", unique_map)
            print("Reverse Unique Map:", reverse_unique_map)
            print("parent_map:", parent_map)
