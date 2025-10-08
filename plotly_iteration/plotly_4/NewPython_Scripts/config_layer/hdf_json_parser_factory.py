from single_hdf_json_parser import SingleHDFJSONParser
from multi_hdf_json_parser import MultiHDFJSONParser

class HDFJSONParserFactory:
    @staticmethod
    def create_parser(hdf_file_type, json_file):
        if hdf_file_type == "HDF_WITH_ALLSENSOR":
            return SingleHDFJSONParser(json_file)
        elif hdf_file_type == "HDF_PER_SENSOR":
            return MultiHDFJSONParser(json_file)
        else:
            raise ValueError("Invalid HDF_FILE type in XML configuration.")
