import h5py
# import numpy as np
import os
from b_db_layer.data_mapper import stla, honda ,stla_h
from c_business_layer.data_model import DataContainer

class PersensorHdfParser:
    """Parser for HDF5 files based on an address map and customer type."""
    
    def __init__(self, address_map, customer):
        self.address_map = address_map
        self.customer = customer
        self.data_container = {}
        self.data_container_out = {}
        self.unique_map = {}
        self.unique_map_out = {}
        print(f"Customer: {self.customer}")

    def parse(self):
        """Parse input and output HDF5 files based on the address map."""
        data_mapper,data_mapper_h = self._get_data_mapper()
        
        for input_file, output_file in self.address_map.items():
            # Process input file
            with h5py.File(input_file, 'r') as hdf_file:
                scan_index = hdf_file[f"{data_mapper_h['Header']}/{data_mapper_h['ScanIndex']}"][()]
                # self.data_container = {j: np.empty((0,)) for j in scan_index}
                self.data_container = {j: [] for j in scan_index}
                self._process_data(hdf_file, data_mapper, self.data_container, self.unique_map)

            # Process output file
            with h5py.File(output_file, 'r') as hdf_file:
                scan_index = hdf_file[f"{data_mapper_h['Header']}/{data_mapper_h['ScanIndex']}"][()]
                # self.data_container_out = {j: np.empty((0,)) for j in scan_index}
                self.data_container_out = {j: [] for j in scan_index}
                self._process_data(hdf_file, data_mapper, self.data_container_out, self.unique_map_out)

            # Check if unique maps match
            if self.unique_map == self.unique_map_out:
                print("All data is mapped correctly.")
            else:
                missing_keys = set(self.unique_map.keys()).difference(self.unique_map_out.keys())
                print(f"Some data is missing: {missing_keys}")

            # Generate HTML name from input and output file names
            input_name = os.path.basename(input_file).split('.')[0]  # Get 'inputFL'
            output_name = os.path.basename(output_file).split('.')[0]  # Get 'outputFL'
            html_name = f"{input_name}_{output_name}.html"
            DataContainer(self.data_container, self.unique_map, self.data_container_out, self.unique_map_out, html_name)

    def _get_data_mapper(self):
        """Retrieve the appropriate data mapper based on the customer type."""
        if self.customer == "STLA":
            return stla ,stla_h
        elif self.customer == "HONDA":
            return honda,honda_h
        else:
            raise ValueError("Unsupported customer type.")

    def _process_data(self, hdf_file, data_mapper ,data_cont, uni_map):
        """Process the data from the HDF5 file using NumPy arrays."""
        for io, (key, val) in enumerate(data_mapper.items()):
            if f"{io}_None" not in uni_map:
                uni_map[f"{io}_None"] = key[0]
                
            for jo, j in enumerate(val):
                a = hdf_file[f'data/{str(key[1])}/{str(j[1])}'][()]
                
                for k, l in zip(a, self.data_container):
                    if jo == 0:
                        # data_cont[str(k)] = np.append(data_cont[str(k)],np.array([k]))
                        
                        data_cont[l].append([k])  # Start a new sublist with the first value
                    else:
                        # data_cont[str(k)] = np.append(data_cont[str(k)][-1],k) # Append to the last subarray
                        data_cont[l][-1].append(k)  # Append to the last sublist
                
                    if f"{io}_{jo}" not in uni_map:
                        uni_map[f"{io}_{jo}"] = j[0]
                    # else:
                    #     print("Exiting code: there is an error with unique mapping.")

