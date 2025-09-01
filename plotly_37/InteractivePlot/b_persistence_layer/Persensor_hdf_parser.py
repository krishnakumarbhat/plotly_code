import h5py
import os
from InteractivePlot.c_business_layer.data_model import DataContainer
from InteractivePlot.c_business_layer import hdf_parser

class PersensorHdfParser:
    """Parser for HDF5 files based on an address map and customer type."""
    
    def __init__(self, address_map):
        self.address_map = address_map
        self.data_container_in = {}
        self.data_container_out = {}
        self.val_sig_map_in = {}
        self.sig_val_map_in = {}
        self.val_sig_map_out = {}
        self.sig_val_map_out = {}
        self.html_name =""

    def parse(self):
        """Parse input and output HDF5 files based on the address map."""
        
        for input_file, output_file in self.address_map.items():
            # Process input file
            with h5py.File(input_file, 'r') as hdf_file:
                scan_index =  hdf_file["Header/ScanIndex"][()]
                data_group =  hdf_file["data"]
                self.data_container_in = {j: [] for j in scan_index}
                self.data_container_in ,self.val_sig_map_in,self.sig_val_map_in  = hdf_parser.HDF5Parser.parse(data_group, self.data_container_in, scan_index)
            print(f"Input Data: {self.data_container_in}, Value Signal Map In: {self.val_sig_map_in}")
            # Process output file
            with h5py.File(output_file, 'r') as hdf_file_out:
                scan_index = hdf_file_out["Header/ScanIndex"][()]
                data_group =  hdf_file_out["data"]
                self.data_container_out = {j:[] for j in scan_index}

                self.data_container_out, self.val_sig_map_out ,self.sig_val_map_out  = hdf_parser.HDF5Parser.parse(data_group, self.data_container_out,scan_index)

            print(f"Output Data: {self.data_container_out}, Value Signal Map Out: {self.val_sig_map_out}")

            # Check if unique maps match
            # if self.val_sig_map_in == self.val_sig_map_out:
            #     print("All data is mapped correctly.")
            # else:
            #     missing_keys = set(self.val_sig_map_in.keys()).difference(self.val_sig_map_out.keys())
            #     print(f"Some data is missing: {missing_keys}")

            # Generate HTML name from input and output file names
            input_name = os.path.basename(input_file).split('.')[0]
            output_name = os.path.basename(output_file).split('.')[0]
            self.html_name  = f"{input_name}_{output_name}.html"
            DataContainer( self.data_container_in, self.val_sig_map_in,self.sig_val_map_in, self.data_container_out, self.val_sig_map_out,self.sig_val_map_out, self.html_name )
