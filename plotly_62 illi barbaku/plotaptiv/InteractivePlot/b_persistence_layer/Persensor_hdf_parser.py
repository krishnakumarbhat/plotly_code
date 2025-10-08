import h5py
import os
from InteractivePlot.d_business_layer.data_model import DataContainer
from InteractivePlot.b_persistence_layer import hdf_parser
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage

class PersensorHdfParser:
    """Parser for HDF5 files based on an address map and customer type."""
    
    def __init__(self, address_map):
        self.address_map = address_map
        self.html_name = ""
        self.input_storage = DataModelStorage()
        self.output_storage = DataModelStorage()

    def parse(self):
        """Parse input and output HDF5 files based on the address map."""
        
        for input_file, output_file in self.address_map.items():
            # Process input file
            with h5py.File(input_file, 'r') as hdf_file:
                scan_index =  hdf_file["Header/ScanIndex"][()]
                data_group =  hdf_file["data"]
                self.input_storage.initialize(scan_index)
                hdf_parser.HDF5Parser.parse(data_group, self.input_storage, scan_index)
            
            # Process output file
            with h5py.File(output_file, 'r') as hdf_file_out:
                scan_index = hdf_file_out["Header/ScanIndex"][()]
                data_group =  hdf_file_out["data"]
                self.output_storage.initialize(scan_index)
                hdf_parser.HDF5Parser.parse(data_group, self.output_storage, scan_index)

            # Generate HTML name from input and output file names
            input_name = os.path.basename(input_file).split('.')[0]
            output_name = os.path.basename(output_file).split('.')[0]
            self.html_name  = f"{input_name}_{output_name}.html"
            
            DataContainer(
                self.input_storage,
                self.output_storage,
                self.html_name
            )
