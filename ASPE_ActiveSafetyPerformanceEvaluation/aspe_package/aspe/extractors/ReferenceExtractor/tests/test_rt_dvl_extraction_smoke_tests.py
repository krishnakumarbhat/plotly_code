# coding utf-8
"""
Main testing file for Rt DVL Extraction smoke tests
"""
import pytest

from aspe.extractors.API.dvl import parse_dvl
from aspe.extractors.ReferenceExtractor.RtDVLExtractor import RtDVLExtractor
from aspe.utilities.SupportingFunctions import get_default_rt_3000_dbc_path, get_logs_catalog

catalog_path = r'\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\ReferenceExtractor\RtDVLExtractor\smoke_tests'
logs_extension = '.dvl'


@pytest.mark.parametrize('log_path', get_logs_catalog(catalog_path, logs_extension))
def test_rt_dvl_example_extraction(path_provider, log_path):
    if 'FTP500_TC2_D30_134857_001_rRf360t4000304v202r1p50' in log_path:
        # TODO fix in CEA-246
        pytest.skip()

    if 'RNASUV_SRR5_K0402B_20181112_201_MO_TC1_80_L_001_rRf360t4010304v202r1p50' in log_path:
        # TODO fix in CEA-248
        pytest.skip()

    # Setup
    log_path = path_provider.get_log_path(log_path)

    # Parse
    parsed_data = parse_dvl(log_path, dbc_config={4: get_default_rt_3000_dbc_path()})

    # Verify
    rt_dvl_extractor = RtDVLExtractor()
    rt_dvl_extractor.extract_data(parsed_data)
    assert True
