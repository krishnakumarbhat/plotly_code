from typing import List

import numpy as np
import pytest

from aspe.utilities.SupportingFunctions import get_f360_mudp_resim_log_path
from aspe.utilities.SupportingFunctions import get_nested_dict_keys
from aspe.parsers.testing import assert_nested_dict_equal, assert_mudp_parsed_data_equal


@pytest.mark.parametrize('d, expected_output', [
    ({}, []),

    ({
        'a': 1,
        'b': 2,
        'c': 3,
     },
     [
        ('a',),
        ('b',),
        ('c',),
    ]),

    ({
         'a': 1,
         'b': {
             '1': 2,
             '2': 3,
         },
         'c': {
             '1': {
                 'A': 4,
                 'B': 5,
             },
             '2': {
                 'A': 6,
             },
             '3': 7,
         }
     },
     [
         ('a',),
         ('b', '1'),
         ('b', '2'),
         ('c', '1', 'A'),
         ('c', '1', 'B'),
         ('c', '2', 'A'),
         ('c', '3'),
    ]),
])
def test_get_nested_dict_keys(d: dict, expected_output: List[tuple]):
    nested_keys_iter = get_nested_dict_keys(d)
    assert list(nested_keys_iter) == expected_output


@pytest.mark.parametrize('before, after, missing_signal_names', [
    (
        {'speed': None, 'heading': None},
        {'speed': None, 'heading': None, 'curvature': None},
        ['curvature'],
    ),
    (
        {'speed': None, 'heading': None, 'curvature': None},
        {'speed': None, 'heading': None},
        ['curvature'],
    ),
    (
        {'obj': {'speed': None, 'heading': None, 'curvature': None}},
        {'obj': {'speed': None, 'heading': None}},
        ['obj.curvature'],
    ),
    (
        {'obj': {'speed': None, 'obj2': {'heading': None, 'obj3': {'curvature': None}}}},
        {'obj': {'speed': None, 'heading': None}},
        ['obj2.heading', 'obj3.curvature'],
    ),
])
def test_assert_nested_dict_equal_signal_not_found(before: dict, after: dict, missing_signal_names: List[str]):
    with pytest.raises(AssertionError) as excinfo:
        assert_nested_dict_equal(before, after)
    assert all(map(str(excinfo.value).__contains__, missing_signal_names + ['not found']))


@pytest.mark.parametrize('before, after, different_signal_names', [
    (
        {'speed': np.array([1, 2, 3])},
        {'speed': np.array([1, 2, 4])},
        ['speed'],
    ),
    (
        {'speed': np.array([1, 2, 3]), 'heading': np.array([1, 2, 3]), 'curvature': np.array([1, 2, 3])},
        {'speed': np.array([1, 2, 4]), 'heading': np.array([1, 2, 3]), 'curvature': np.array([1, 2, 4])},
        ['speed', 'curvature'],
    ),
])
def test_assert_nested_dict_equal_signal_values_different(before: dict, after: dict, different_signal_names: List[str]):
    with pytest.raises(AssertionError) as excinfo:
        assert_nested_dict_equal(before, after)
    assert all(map(str(excinfo.value).__contains__, different_signal_names + ['different values']))


@pytest.mark.parametrize('stream_data', [
    {},
    {'speed': np.array([1, 2, 3])},
    {'speed': np.array([1, 2, 3]), 'heading': np.array([1, 2, 3]), 'curvature': np.array([1, 2, 3])},
])
def test_assert_nested_dict_equal_passes(stream_data: dict):
    assert_nested_dict_equal(stream_data, stream_data)


@pytest.mark.parametrize('before, after, missing_stream_numbers', [
    (
        {'parsed_data': {1: None, 2: None}},
        {'parsed_data': {1: None, 2: None, 3: None}},
        [3],
    ),
    (
        {'parsed_data': {1: None, 2: None, 3: None}},
        {'parsed_data': {1: None, 2: None}},
        [3],
    ),
    (
        {'parsed_data': {1: None, 2: None, 3: None, 4: None, 5: None}},
        {'parsed_data': {1: None, 2: None}},
        [3, 4, 5],
    ),
])
def test_assert_mudp_parsed_data_equal_stream_not_found(before: dict, after: dict, missing_stream_numbers: List[int]):
    with pytest.raises(AssertionError) as excinfo:
        assert_mudp_parsed_data_equal(before, after)
    assert all(map(str(excinfo.value).__contains__, map(str, missing_stream_numbers)))


@pytest.mark.parametrize('original_log_path, resim_extension, expected_output', [
    (
        r"C:\path\to\dir\log.dvl",
        'resim_extension',
        r"C:\path\to\dir\resim_extension\log_resim_extension.mudp",
    ),
    (
        r"\\10.224.186.68\AD-Shared\F360\Logs\Golden_Set_For_Refactoring\PSA\FTP402_TC4_TA_154934_001.dvl",
        'rRf360t4130309v205p50_DFT_2_18_01',
        r"\\10.224.186.68\AD-Shared\F360\Logs\Golden_Set_For_Refactoring\PSA\rRf360t4130309v205p50_DFT_2_18_01\FTP402_TC4_TA_154934_001_rRf360t4130309v205p50_DFT_2_18_01.mudp",
    ),
    (
        r"FTP402_TC4_TA_154934_001.dvl",
        'rRf360t4130309v205p50_DFT_2_18_01',
        r"rRf360t4130309v205p50_DFT_2_18_01\FTP402_TC4_TA_154934_001_rRf360t4130309v205p50_DFT_2_18_01.mudp",
    ),
])
def test_get_mudp_resim_log_path(original_log_path: str, resim_extension: str, expected_output: str):
    assert get_f360_mudp_resim_log_path(original_log_path, resim_extension) == expected_output
