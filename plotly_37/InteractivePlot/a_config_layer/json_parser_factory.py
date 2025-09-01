from .allsensor_json_parser import AllsensorJSONParser
from .persensor_json_parser import PersensorJSONParser

class JSONParserFactory:
    @staticmethod
    def create_parser(hdf_file_type, json_file):
        if hdf_file_type == "HDF_WITH_ALLSENSOR":
            return AllsensorJSONParser(json_file)
        elif hdf_file_type == "HDF_PER_SENSOR":
            return PersensorJSONParser(json_file)
        else:
            raise ValueError("Invalid HDF_FILE type in XML configuration.")
