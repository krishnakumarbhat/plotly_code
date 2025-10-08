# coding=utf-8
"""
File with examples for F360 mudp files.
"""

from aspe.extractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor
from aspe.parsers.MudpParser import MudpParser
from aspe.parsers.MudpParser.f360_defaults_configs import source_to_parse, streams_to_read, unknown_size_per_stream
from aspe.utilities.SupportingFunctions import save_to_pkl


def example_from_mudp(mudp_log_path, mudp_stream_def_path, save_to_pickle=False):
    """
    Basic example with direct mudp input file.
    Getting raw mudp file, parsing and generating extracted data
    :param mudp_log_path: path to mudp file
    :param mudp_stream_def_path: path to mudp stream definition
    :param save_to_pickle: save output to pickle file
    :return: Extracted Data
    """

    # Parsing
    parser = MudpParser(streams_to_read=streams_to_read, source_to_parse=source_to_parse,
                        unknown_size_per_stream=unknown_size_per_stream,
                        mudp_stream_def_path=mudp_stream_def_path)
    parsed_data = parser.parse(mudp_log_path)

    # Extracting
    f360_extractor_object = F360MUDPExtractor()
    f360_extracted_data = f360_extractor_object.extract_data(parsed_data)

    if save_to_pickle:
        save_path = mudp_log_path.replace('.mudp', '_MUDP_Extracted.pickle')
        save_to_pkl(f360_extracted_data, save_path)

    return f360_extracted_data


if __name__ == "__main__":
    input_mudp_stream_def_path = r'C:\bd\OT_ObjectTracking\modules\F360Core\sw\zResimSupport\stream_definitions'
    input_mudp_log_path = r"C:\Users\tzk56p\Documents\01_Projects\F360\logs\9p5_checkout\rRf360t9050309v205p50\stream70_v5_example_log\HGR_Tracker_Checkout_9p05p00_1xFLR4plus_20240118_111023_007_rRf360t9050309v205p50.mudp"

    data = example_from_mudp(input_mudp_log_path, input_mudp_stream_def_path, save_to_pickle=False)
    apa=1