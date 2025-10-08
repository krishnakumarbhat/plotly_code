from aspe.extractors.ENV.Keg.ENVKegExtractor import ENVKegExtractor
from aspe.extractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor
from aspe.extractors.ReferenceExtractor.RtDVLExtractor import RtDVLExtractor
from aspe.parsers.DvlParser.dvl_parser import DVLParser
from aspe.parsers.KegParser.keg_parser import KegParser
from aspe.parsers.MudpParser.f360_defaults_configs import streams_to_read, unknown_size_per_stream
from aspe.parsers.MudpParser.mudp_parser import MudpParser


class F360MudpExtractionPipeline:
    def __init__(self, mudp_stream_def_path=None, extractor_kwargs={}):
        self.mudp_stream_def_path = mudp_stream_def_path
        self.extractor = F360MUDPExtractor(**extractor_kwargs)
        self.parser = MudpParser(streams_to_read, unknown_size_per_stream, self.mudp_stream_def_path)

    def process(self, data_path):
        parsed_data = self._parse_data(data_path)
        extracted_data = self._extract_data(parsed_data)
        return extracted_data

    def _parse_data(self, data_path):
        print('Parsing .mudp data ...')
        parsed_data = self.parser.parse(data_path)
        return parsed_data

    def _extract_data(self, parsed_data):
        print('Extracting .mudp data ...')
        extracted_data = self.extractor.extract_data(parsed_data)
        return extracted_data


class RtRangeDvlExtractionPipeline:
    def __init__(self, dbc_config: dict, msg_sets_config: dict):
        self.extractor = RtDVLExtractor(msg_sets_config=msg_sets_config)
        self.parser = DVLParser(dbc_config=dbc_config)

    def process(self, data_path):
        parsed_data = self._parse_data(data_path)
        extracted_data = self._extract_data(parsed_data)
        return extracted_data

    def _parse_data(self, data_path):
        print('Parsing .dvl data ...')
        parsed = self.parser.parse(data_path)
        return parsed

    def _extract_data(self, parsed_data):
        print('Extracting .dvl data ...')
        extracted_data = self.extractor.extract_data(parsed_data)
        return extracted_data


if __name__ == '__main__':
    mudp_stream_def_path = r'\\10.224.186.68\AD-Shared\ASPE\configurations\F360\MUDP_Stream_Definitions\stream_definitions'

    mudp_pipeline = F360MudpExtractionPipeline(mudp_stream_def_path)
    dvl_pipeline = RtRangeDvlExtractionPipeline()

    mudp_log_path = r'\\10.224.186.68\AD-Shared\ASPE\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA_FTP_402\rRf360t4060306v204p50\FTP402_TC1_10_150435_001_rRf360t4060306v204p50.mudp'
    dvl_log_path = r'\\10.224.186.68\AD-Shared\ASPE\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA_FTP_402\rRf360t4060306v204p50\FTP402_TC1_10_150435_001_rRf360t4060306v204p50.dvl'

    mudp_data = mudp_pipeline.process(mudp_log_path)
    dvl_data = dvl_pipeline.process(dvl_log_path)
