from InteractivePlot.b_persistence_layer.allsensor_hdf_parser import AllsensorHdfParser
from InteractivePlot.b_persistence_layer.Persensor_hdf_parser import PersensorHdfParser
import h5py
import os

class HdfProcessorFactory:
    """
    Factory class for handling different types of HDF5 file formats and creating appropriate parsers.
    
    This class is part of the persistence layer and serves as a bridge between the raw data (HDF5 files)
    and the business layer. It determines which parser to use based on the HDF file type and handles
    the extraction of metadata such as available sensors.
    
    The factory supports two main HDF file formats:
    1. HDF_WITH_ALLSENSOR - A single HDF file containing data for all sensors
    2. HDF_PER_SENSOR - Separate HDF files for each sensor (typically with FL, FR, RL, RR suffixes)
    """
    
    def __init__(self, file_dict, hdf_file_type, output_dir):
        """
        Initialize the HDF processor factory.
        
        Parameters:
            file_dict (dict): Dictionary mapping input file paths to output file paths
            hdf_file_type (str): Type of HDF file format ("HDF_WITH_ALLSENSOR" or "HDF_PER_SENSOR")
            output_dir (str): Directory to save the generated output files
        """
        self.file_dict = file_dict
        self.hdf_file_type = hdf_file_type
        self.output_dir = output_dir
        self.available_sensors = self.get_available_sensors()

    def get_available_sensors(self):
        """
        Extract available sensors from the HDF files based on the file format.
        
        This method analyzes the structure of the HDF5 files to identify the available sensors,
        which can be used for filtering and processing specific sensor data.
        
        For all-sensor format, sensors are identified as top-level groups in the HDF file.
        For per-sensor format, sensors are extracted from filenames (FL, FR, RL, RR) or 
        from the data structure if available.
        
        Returns:
            list: List of available sensor names found in the HDF files
        """
        # sensors = set()
        sensors = {}
        
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
                            sensors[key] = None
                elif self.hdf_file_type == "HDF_PER_SENSOR":
                    # For per-sensor format, extract from filename or try to find in data
                    input_name = os.path.basename(input_file)
                    if input_name.endswith(('FL.h5', 'FR.h5', 'RL.h5', 'RR.h5')):
                        # Extract from filename (e.g., "radar_FL.h5" → "FL")
                        sensors[input_name[-6:-4]]= None

                    else:
                        # Try to extract from data structure if available
                        if "data" in hdf_file:
                            data_group = hdf_file["data"]
                            for key in data_group.keys():
                                if isinstance(data_group[key], h5py.Group):
                                    sensors[key]=None
        except Exception as e:
            print(f"Warning: Could not extract sensor information: {e}")
            
        return list(sensors.keys())

    def process(self):
        """
        Process the HDF files by creating and using appropriate parsers based on the file type.
        
        This method:
        1. Selects the appropriate parser class based on the HDF file type
        2. Creates an instance of the parser with the input/output file mapping
        3. Delegates the parsing process to the selected parser
        
        Returns:
            The result from the parser's parse method (typically processed data)
            
        Raises:
            ValueError: If an invalid HDF file type is specified
        """
        address_map = self.file_dict
        file_type_name = self.hdf_file_type

        if file_type_name == "HDF_PER_SENSOR":
            parser_wrapper = PersensorHdfParser(address_map, self.output_dir)
            return parser_wrapper.parse()
        elif file_type_name == "HDF_WITH_ALLSENSOR":
            parser_wrapper = AllsensorHdfParser(address_map, self.output_dir)
            return parser_wrapper.parse()
        else:
            raise ValueError("Invalid HDF_FILE type in XML configuration.")
