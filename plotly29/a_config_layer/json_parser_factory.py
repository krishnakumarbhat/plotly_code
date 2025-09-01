from a_config_layer.allsensor_json_parser import AllsensorJSONParser
from a_config_layer.persensor_json_parser import PersensorJSONParser

class JSONParserFactory:
    @staticmethod
    def create_parser(hdf_file_type, json_file):
        if hdf_file_type[0] == "HDF_WITH_ALLSENSOR":
            return AllsensorJSONParser(json_file)
        elif hdf_file_type[0] == "HDF_PER_SENSOR":
            return PersensorJSONParser(json_file)
        else:
            raise ValueError("Invalid HDF_FILE type in XML configuration.")
