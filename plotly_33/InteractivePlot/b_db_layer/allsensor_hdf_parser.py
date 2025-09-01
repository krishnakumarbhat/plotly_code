import h5py
import os

from c_business_layer.data_model import DataContainer

class AllsensorHdfParser:
    """Parser for HDF5 files based on an address map and customer type."""
    
    def __init__(self, address_map):
        self.address_map = address_map
        self.data_container_in = {}
        self.data_container_out = {}
        self.val_sig_map_in = {}
        self.val_sig_map_out = {}


    def parse(self):
        """Parse input and output HDF5 files based on the address map."""
        lis = ["RL","RR","FL","FR"]
        for input_file, output_file in self.address_map.items():
            # Process input file

            for sen in lis:
                
                with h5py.File(input_file, 'r') as hdf_file:
                    CustomerID = hdf_file[f"*/Header/CustomerID"][()]
                    if CustomerID ==None:
                        print("CustomerID is not their in Header see the spelling and caps")
                    data_mapper,data_mapper_h = self._get_data_mapper(CustomerID)
                    scan_index = hdf_file[f"{sen}/{data_mapper_h['Header']}/{data_mapper_h['ScanIndex']}"][()]
                    # self.data_container_in = {j: np.empty((0,)) for j in scan_index}
                    self.data_container_in = {j: [] for j in scan_index}
                    self._process_data(hdf_file, data_mapper, self.data_container_in, self.val_sig_map_in)

                # Process output file
                with h5py.File(output_file, 'r') as hdf_file:
                    CustomerID = hdf_file[f"Header/CustomerID"][()]
                    if CustomerID ==None:
                        print("CustomerID is not their in Header see the spelling and caps")
                    data_mapper,data_mapper_h = self._get_data_mapper(CustomerID)
                    scan_index = hdf_file[f"{data_mapper_h['Header']}/{data_mapper_h['ScanIndex']}"][()]
                    # self.data_container_out = {j: np.empty((0,)) for j in scan_index}
                    self.data_container_out = {j: [] for j in scan_index}
                    self._process_data(hdf_file, data_mapper, self.data_container_out, self.val_sig_map_out)

                # Check if unique maps match
                if self.val_sig_map_in == self.val_sig_map_out:
                    print("All data is mapped correctly.")
                else:
                    missing_keys = set(self.val_sig_map_in.keys()).difference(self.val_sig_map_out.keys())
                    print(f"Some data is missing: {missing_keys}")

                # Generate HTML name from input and output file names
                input_name = os.path.basename(input_file).split('.')[0]
                output_name = os.path.basename(output_file).split('.')[0]
                html_name = f"{input_name}_{sen}_{output_name}_{sen}.html"
                DataContainer(self.data_container_in, self.val_sig_map_in, self.data_container_out, self.val_sig_map_out, html_name)

    # def _get_data_mapper(self,CustomerID):
    #     """Retrieve the appropriate data mapper based on the customer type."""
        
    #     if CustomerID == b'stla':
    #         return stla ,stla_h
    #     elif CustomerID == b'HONDA':
    #         return honda,honda_h
    #     else:
    #         raise ValueError("Unsupported customer type.")

    # def _process_data(self, hdf_file, data_mapper ,data_cont, uni_map):
    #     """Process the data from the HDF5 file using NumPy arrays."""
    #     for io, (key, val) in enumerate(data_mapper.items()):
    #         if f"{io}_None" not in uni_map:
    #             uni_map[f"{io}_None"] = key[0]
                
    #         for jo, j in enumerate(val):
    #             a = hdf_file[f'data/{str(key[1])}/{str(j[1])}'][()]
                
    #             for k, l in zip(a, self.data_container_in):
    #                 if jo == 0:
    #                     # data_cont[str(k)] = np.append(data_cont[str(k)],np.array([k]))
                        
    #                     data_cont[l].append([k])  # Start a new sublist with the first value
    #                 else:
    #                     # data_cont[str(k)] = np.append(data_cont[str(k)][-1],k) # Append to the last subarray
    #                     data_cont[l][-1].append(k)  # Append to the last sublist
                
    #                 if f"{io}_{jo}" not in uni_map:
    #                     uni_map[f"{io}_{jo}"] = j[0]
    #                 # else:
    #                 #     print("Exiting code: there is an error with unique mapping.")

