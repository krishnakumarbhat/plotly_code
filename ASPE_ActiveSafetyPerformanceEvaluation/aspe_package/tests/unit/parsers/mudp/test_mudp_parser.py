"""
mudp_parser.py
**************
Parser for mudp binary files.
"""
import pytest

from aspe.parsers.MudpParser.mudp_parser import MudpParser


@pytest.mark.parametrize('mudp_stream_def_path', (
        None,
        r'\\10.224.186.68\AD-Shared\ASPE\configurations\F360\MUDP_Stream_Definitions\stream_definitions'
))
def test_smoke_test(mudp_stream_def_path):
    filename = r''
    mudp_parser_config_path = r'aspe.parsers\MudpParser\test\mudp_data_parser_config.json'
    MudpParser(filename, mudp_parser_config_path, mudp_stream_def_path)
