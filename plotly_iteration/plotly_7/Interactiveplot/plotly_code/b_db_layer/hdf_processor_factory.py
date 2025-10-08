from b_db_layer.allsensor_hdf_parser import AllsensorHdfParser
from b_db_layer.Persensor_hdf_parser import PersensorHdfParser

class HdfProcessorFactory:
    def __init__(self, file_dict ,hdf_file_type):
        self.file_dict = file_dict
        self.hdf_file_type = hdf_file_type

    def process(self):
        address_map =  self.file_dict
        file_type_name = self.hdf_file_type[0]
        customer = self.hdf_file_type[1]

        if file_type_name == "HDF_PER_SENSOR":
            parser_wrapper = PersensorHdfParser(address_map, customer)
            return parser_wrapper.parse()
        else:
            parser_wrapper = AllsensorHdfParser(address_map, customer)
            return parser_wrapper.parse()
