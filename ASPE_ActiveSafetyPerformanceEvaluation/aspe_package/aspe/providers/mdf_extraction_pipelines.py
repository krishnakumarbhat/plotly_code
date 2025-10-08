from mdf_parser_pkg import mdf_parser

from aspe.extractors.API.mdf import extract_f360_bmw_mid_from_mf4
from aspe.extractors.F360.MDF4_BMW_mid.F360Mdf4BmwExtractor import F360Mdf4BmwExtractor
from aspe.extractors.ReferenceExtractor.RtRangeMdfExtractor import RtRangeMdfExtractor
from aspe.utilities.mf4_rtrange_extractor import parse_rt


class F360MdfExtractionPipeline:
    parser = staticmethod(extract_f360_bmw_mid_from_mf4)
    def __init__(self, dbc=None, extractor_kwargs={}):
        self.dbc = dbc
        self.extractor = F360Mdf4BmwExtractor(f_extract_raw_signals=True, **extractor_kwargs)
        # self.parser = extract_f360_bmw_mid_from_mf4()

    def process(self, data_path):
        parsed_data = self._parse_data(data_path)
        extracted_data = self._extract_data(parsed_data)
        return extracted_data

    def _parse_data(self, data_path):
        print('Parsing .mdf data ...')
        parsed_data = self.parser(data_path)
        return parsed_data

    def _extract_data(self, parsed_data):
        print('Extracting .mdf data ...')
        extracted_data = self.extractor.extract_data(parsed_data)
        return extracted_data


class RtRangeMdfExtractionPipeline:
    def __init__(self):
        self.extractor = RtRangeMdfExtractor()
        self.parser = parse_rt

    def process(self, data_path):
        parsed_data = self._parse_data(data_path)
        extracted_data = self._extract_data(parsed_data)
        return extracted_data

    def _parse_data(self, data_path):
        print('Parsing .mdf data ...')
        parsed = self.parser(data_path)
        return parsed

    def _extract_data(self, parsed_data):
        print('Extracting .mdf data ...')
        extracted_data = self.extractor.extract_data(parsed_data)
        return extracted_data

