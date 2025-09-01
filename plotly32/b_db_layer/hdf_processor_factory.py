from b_db_layer.allsensor_hdf_parser import AllsensorHdfParser
from b_db_layer.Persensor_hdf_parser import PersensorHdfParser

class HdfProcessorFactory:
    def __init__(self, file_dict ,hdf_file_type):
        self.file_dict = file_dict
        self.hdf_file_type = hdf_file_type

    def process(self):
        address_map =  self.file_dict
        file_type_name = self.hdf_file_type

        if file_type_name == "HDF_PER_SENSOR":
            parser_wrapper = PersensorHdfParser(address_map)
            return parser_wrapper.parse()
        elif file_type_name == "HDF_WITH_ALLSENSOR":
            parser_wrapper = AllsensorHdfParser(address_map)
            return parser_wrapper.parse()
        else:
            raise ValueError("Invalid HDF_FILE type in XML configuration.")
