from aspe.extractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor
from aspe.extractors.ReferenceExtractor.RtDVLExtractor import RtDVLExtractor
from aspe.parsers import MudpParser
from aspe.parsers.DvlParser import DVLParser


def get_estimated_and_reference_data(mudp_log_path, dvl_log_path, mudp_parser_config_path, dvl_parser_config_path,
                                     mudp_stream_def_path=None):
    dvl_parser = DVLParser(dvl_log_path, dvl_parser_config_path)
    dvl_parser.decode()
    dvl_parser.extract_message_sets()

    mudp_parser = MudpParser.MudpHandler(mudp_log_path, mudp_parser_config_path, mudp_stream_def_path)
    mudp_parsed_data = mudp_parser.decode()

    # Extract
    rt_extractor = RtDVLExtractor()
    rt_extracted_data = rt_extractor.extract_data(dvl_parser.__dict__)

    f360_extractor_object = F360MUDPExtractor()
    f360_extracted_data = f360_extractor_object.extract_data(mudp_parsed_data)
    return f360_extracted_data, rt_extracted_data


if __name__ == '__main__':
    mudp_log_path = (r"\\10.224.186.68\AD-Shared\ASPE\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA_FTP_402"
                     r"\rRf360t4060306v204p50\FTP402_TC1_90_151039_001_rRf360t4060306v204p50.mudp")
    dvl_log_path = (r"\\10.224.186.68\AD-Shared\ASPE\Logs\F360\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA_FTP_402"
                    r"\rRf360t4060306v204p50\FTP402_TC1_90_151039_001_rRf360t4060306v204p50.dvl")
    mudp_parser_config_path = \
        r"\\10.224.186.68\AD-Shared\ASPE\configurations\parsers_config\mudp_data_parser_config.json"
    dvl_parser_config_path = r"\\10.224.186.68\AD-Shared\ASPE\configurations\parsers_config\dvl_data_parser_config.json"
    mudp_stream_def_path = \
        r"\\10.224.186.68\AD-Shared\ASPE\configurations\F360\MUDP_Stream_Definitions\stream_definitions"
    get_estimated_and_reference_data(mudp_log_path, dvl_log_path, mudp_parser_config_path, dvl_parser_config_path,
                                     mudp_stream_def_path)
