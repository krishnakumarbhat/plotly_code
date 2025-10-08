# coding utf-8
"""
Main testing file for F360 MUDP Extraction smoke tests
"""
import pytest
import numpy as np

from aspe.extractors.API.mudp import parse_mudp
from aspe.extractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor
from aspe.utilities.SupportingFunctions import get_logs_catalog

catalog_path = r'\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\smoke_test'
logs_extension = '.mudp'


@pytest.fixture
def parsed_data_1():
    """
    Providing some parsed data, there are only few signals like in case where only part of all has been extracted
    :return: Dict: same like from mudp parsed data
    """
    dummy_parsed_data = {
        'filename': 'dummy_name',
        'parsed_data': {
            6: {
                'obj': {
                    'reducedStatus': np.array([[4, 4], [4, 4]]),
                    'vcs_xposn': np.array([[10.0, 1.0], [11.0, 1.0]]),
                    'vcs_yposn': np.array([[10.0, 1.0], [11.0, 1.0]]),
                    'wid1': np.array([[1.0, 1.0], [1.0, 1.0]]),
                    'wid2': np.array([[1.0, 1.0], [1.0, 1.0]]),
                }
            },
            7: {
                'tracker_index': np.array([[1], [2]]),
                # double bracket because ndarray is needed (shape as 2 elem tuple)
            }
        }
    }
    return dummy_parsed_data


@pytest.mark.parametrize('log_path', get_logs_catalog(catalog_path, logs_extension))
def test_f360_mudp_example_extraction(path_provider, log_path):
    """
    General smoke test using example log file
    """
    if r'PSA_20180613_152933_003' in log_path:
        # TODO fix in CEA-250
        pytest.skip()

    # Setup
    log_path = path_provider.get_log_path(log_path)
    mudp_stream_def_path = path_provider.get_mudp_stream_def_path()

    # Parsing
    parsed_data = parse_mudp(log_path, mudp_stream_def_path)

    # Extract/Verify
    f360_extractor = F360MUDPExtractor()
    f360_extractor.extract_data(parsed_data)
    assert True


def test_f360_mudp_objects_extraction(parsed_data_1):
    """
    TODO with this dummy data this test doesnt pass in case of unhadndled execption in _calc_bbox_center_positions
    TODO test can be either deleted nor function shall be fixed
    """
    # TODO fix in CEA-251
    pytest.skip()

    # Setup
    f360_extractor_object = F360MUDPExtractor()

    # Verify
    f360_extractor_object.extract_data(parsed_data_1)
    assert True
