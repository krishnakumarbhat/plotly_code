import h5py
import os
from ..c_business_layer import hdf_parser
from ..c_business_layer.data_model import DataContainer
from b_persistence_layer.Persensor_hdf_parser import PersensorHdfParser
class AllsensorHdfParser(PersensorHdfParser):
    """Parser for HDF5 files based on an address map and customer type."""
    def __init__(self, address_map):
        super().__init__(address_map)
        self.html_name = ""

    def parse(self):
        """Parse input and output HDF5 files based on the address map."""
        sensor_list = ["RL","RR","FL","FR"]
        for index, (input_file, output_file) in enumerate(self.address_map.items()):
            for sensor in sensor_list:
                input_file_name = os.path.basename(input_file).replace('.', '')
                output_file_name = os.path.basename(output_file).replace('.', '')
                self.html_name = input_file_name +"_"+ output_file_name + "_" + sensor
                # Process input file
                with h5py.File(input_file, 'r') as hdf_file:
                    scan_index =  hdf_file["Header/ScanIndex"][()]
                    self.data_container_in = {j: [] for j in scan_index}
                    data_group =  hdf_file[f"data/{sensor}"]
                    self.data_container_in ,self.val_sig_map_in , self.sig_val_map_in = hdf_parser.HDF5Parser.parse(data_group, self.data_container_in, self.val_sig_map_in ,scan_index)
                
                print(f"Input Data: {self.data_container_in}, Value Signal Map In: {self.val_sig_map_in}")
                
                # Process output file
                with h5py.File(output_file, 'r') as hdf_file_out:
                    scan_index = hdf_file_out["Header/ScanIndex"][()]
                    data_group =  hdf_file_out[f"data/{sensor}"]
                    self.data_container_out = {j: [] for j in scan_index}
                    # self.address_map.value() = self.address_map.value() + "_" + sensor
                    self.data_container_out, self.val_sig_map_out , self.sig_val_map_in = hdf_parser.HDF5Parser.parse(data_group, self.data_container_out, self.val_sig_map_out ,scan_index)

                print(f"Output Data: {self.data_container_out}, Value Signal Map Out: {self.val_sig_map_out}")
                DataContainer( self.data_container_in, self.val_sig_map_in, self.data_container_out, self.val_sig_map_out, self.html_name)
