from InteractivePlot.b_persistence_layer.allsensor_hdf_parser import AllsensorHdfParser
from InteractivePlot.b_persistence_layer.Persensor_hdf_parser import PersensorHdfParser
import h5py
import os

class HdfProcessorFactory:
    def __init__(self, file_dict, hdf_file_type, output_dir=None, num_processes=None):
        self.file_dict = file_dict
        self.hdf_file_type = hdf_file_type
        self.output_dir = output_dir or "html"  # Default to "html" if not provided
        
        # Set the number of processes to use for parallel processing
        if num_processes is not None:
            try:
                self.num_processes = int(num_processes)
                if self.num_processes <= 0:
                    self.num_processes = None  # Use default
            except ValueError:
                self.num_processes = None  # Use default
        else:
            # Try to get from environment variable, otherwise use None (default)
            try:
                env_processes = os.environ.get('INTERACTIVE_PLOT_MP_PROCESSES')
                if env_processes:
                    self.num_processes = int(env_processes)
                else:
                    self.num_processes = None
            except (ValueError, TypeError):
                self.num_processes = None
        
        # Check if nested multiprocessing is disabled
        self.nested_mp_disabled = os.environ.get('DISABLE_NESTED_MP', '').lower() == 'true'
        
        self.available_sensors = self.get_available_sensors()

    def get_available_sensors(self):
        """Extract available sensors from the HDF files"""
        sensors = set()
        
        if not self.file_dict:
            return list(sensors)
            
        # Get the first input file to extract sensor information
        input_file = next(iter(self.file_dict.keys()))
        
        try:
            with h5py.File(input_file, 'r') as hdf_file:
                if self.hdf_file_type == "HDF_WITH_ALLSENSOR":
                    # For all-sensor HDF format, sensors are top-level groups
                    for key in hdf_file.keys():
                        if key != "Header" and key != "data" and isinstance(hdf_file[key], h5py.Group):
                            sensors.add(key)
                elif self.hdf_file_type == "HDF_PER_SENSOR":
                    # For per-sensor format, extract from filename or try to find in data
                    input_name = os.path.basename(input_file)
                    if input_name.endswith(('FL.hdf', 'FR.hdf', 'RL.hdf', 'RR.hdf')):
                        # Extract from filename
                        sensors.add(input_name[-6:-4])
                    else:
                        # Try to extract from data structure if available
                        if "data" in hdf_file:
                            data_group = hdf_file["data"]
                            for key in data_group.keys():
                                if isinstance(data_group[key], h5py.Group):
                                    sensors.add(key)
        except Exception as e:
            print(f"Warning: Could not extract sensor information: {e}")
            
        return list(sensors)

    def process(self):
        address_map = self.file_dict
        file_type_name = self.hdf_file_type

        if file_type_name == "HDF_PER_SENSOR":
            parser_wrapper = PersensorHdfParser(address_map, self.output_dir)
            return parser_wrapper.parse()
        elif file_type_name == "HDF_WITH_ALLSENSOR":
            parser_wrapper = AllsensorHdfParser(address_map, self.output_dir)
            
            # Configure the parser with our settings
            if hasattr(parser_wrapper, 'set_num_processes') and self.num_processes is not None:
                parser_wrapper.set_num_processes(self.num_processes)
            
            # Configure nested multiprocessing setting
            if hasattr(parser_wrapper, 'set_nested_mp_enabled'):
                parser_wrapper.set_nested_mp_enabled(not self.nested_mp_disabled)
                
            return parser_wrapper.parse()
        else:
            raise ValueError("Invalid HDF_FILE type in XML configuration.")
