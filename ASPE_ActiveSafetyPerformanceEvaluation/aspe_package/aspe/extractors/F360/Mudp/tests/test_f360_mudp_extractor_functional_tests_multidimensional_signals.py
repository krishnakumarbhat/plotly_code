# coding utf-8
"""
Functional tests for F360MUDPExtractor for extracting multi-dimensional signals
"""
import pytest
import pandas as pd
import typing

from aspe.extractors.API.mudp import extract_f360_from_mudp
from aspe.extractors.Interfaces.ExtractedData import ExtractedData


@pytest.fixture(scope='module')
def extracted_data(path_provider) -> ExtractedData:
    """
    Returns example extracted data.
    :return: extracted data
    """
    mudp_log_path = path_provider.get_log_path(r"\\10.224.186.68\AD-Shared\ASPE\Logs\Tests\aspe.extractors\F360\F360MUDPExtractor\functional_test\RNASUV_SRR5_K0402B_20181112_201_MO_TC1_80_L_001_rRf360t4010304v202r1p50.mudp")
    mudp_stream_def_path = path_provider.get_mudp_stream_def_path()

    extracted_data = extract_f360_from_mudp(mudp_log_path, mudp_stream_def_path=mudp_stream_def_path, host=False, internal_objects=True)
    return extracted_data


@pytest.mark.parametrize('dataset_name', [
    'objects',
    'internal_objects'
])
@pytest.mark.parametrize('signal_name, expected_shape', [
    ('motion_model_variances_suppl', (6,)),
    ('motion_model_covariances_suppl', (3,)),
])
def test_signal_has_shape(extracted_data: ExtractedData, dataset_name: str, signal_name: str, expected_shape: typing.Tuple[int]) -> None:
    """
    Tests if a specified signal from a specified dataset in extracted data has correct shape.
    :param extracted_data: output of F360MUDPExtractor.extract_data method
    :param dataset_name: name of dataset to test, e.g. 'objects' or 'internal_objects'
    :param signal_name: name of column to test, e.g. 'motion_model_variances_suppl'
    :param expected_shape: a tuple of integers representing signal value shape, e.g. (3,) or (2,2)
    :return:
    """
    dataset = getattr(extracted_data, dataset_name)
    signal = dataset.signals[signal_name]
    value_has_correct_shape = signal.map(lambda cell: cell.shape == expected_shape)
    all_values_have_correct_shape = pd.Series.all(value_has_correct_shape)
    assert all_values_have_correct_shape
