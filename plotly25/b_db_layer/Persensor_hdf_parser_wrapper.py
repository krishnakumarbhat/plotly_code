# Persensor_hdf_parser_wrapper.py

import h5py
from b_db_layer.hdf_parser import HdfParser
from c_business_layer.data_model import DataContainer

class PersensorHdfParserWrapper:
    def __init__(self, address_map):
        self.address_map = address_map
        self.data_container = DataContainer()  # Create an instance of DataContainer
        
    def parse(self):
        for input_file, output_file in self.address_map.items():
            with h5py.File(input_file, 'r') as fi:
                print(f"Processing input file: {input_file}")
                parsed_data, uni_map, rev_uni_map_sub = HdfParser.bfs_hdf5(fi)  # Capture returned values
                self.data_container.add_data(parsed_data, uni_map, rev_uni_map_sub)  # Store the data
                print("Parsed Data:", parsed_data)
                print("uni_map:", uni_map)
                print("rev_uni_map:", rev_uni_map_sub)

            with h5py.File(output_file, 'r') as fo:
                print(f"Processing output file: {output_file}")
                parsed_data_out, uni_mapout, rev_uni_map_out = HdfParser.bfs_hdf5(fo)  # Capture returned values
                self.data_container.add_data(parsed_data_out, uni_mapout, rev_uni_map_out)  # Store the output data
                print("Parsed Output Data:", parsed_data_out)
                print("uni_map:", uni_mapout)
                print("rev_uni_map", rev_uni_map_out)

        print("Final Data Container:", self.data_container)  # Display final container state
