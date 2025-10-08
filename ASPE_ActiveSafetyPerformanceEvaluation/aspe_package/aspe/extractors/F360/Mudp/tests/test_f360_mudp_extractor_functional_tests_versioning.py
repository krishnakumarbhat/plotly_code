# coding utf-8
"""
Functional tests for versioning handling in F360MUDPExtractor
"""
from dataclasses import dataclass

import pytest

from aspe.extractors.F360.Mudp.Exceptions.MissingStreamException import MissingStreamException
from aspe.extractors.F360.Mudp.Exceptions.VersionNotSupportedException import VersionNotSupportedException
from aspe.extractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor


@pytest.fixture
def parsed_data_1():
    """
    Mock of parsed data with no streams
    :return: parsed_data
    """
    return {
        'path': None,
        'header_data': {}
    }


@pytest.fixture
def parsed_data_2():
    """
    Mock of parsed data with streams 1-20
    :return: parsed_data
    """

    @dataclass
    class Header:
        configuration_file_path: str

    return {
        'path': None,
        'header_data': {
            i: Header(fr'\\127.0.0.1\stream_definitions\strdef_src035_str{i:03d}_ver000.txt')
            for i in range(20)
        },
    }


def test_raise_exc_if_stream_missing(parsed_data_1):
    """
    Tests if MissingStreamException is re-raised by extractor
    when f_raise_exc_if_stream_missing is turned on.
    """
    extractor = F360MUDPExtractor(f_raise_exc_if_stream_missing=True)
    with pytest.raises(MissingStreamException):
        extractor.extract_data(parsed_data_1)


def test_warn_if_stream_missing(parsed_data_1):
    """
    Tests if there is only a warning when there are missing streams in parsed data
    and f_raise_exc_if_stream_missing is turned off.
    """
    extractor = F360MUDPExtractor(f_raise_exc_if_stream_missing=False)
    with pytest.warns(UserWarning):
        extractor.extract_data(parsed_data_1)


def test_raise_exc_if_version_not_supported(parsed_data_2):
    """
    Tests if VersionNotSupportedException is re-raised by extractor
    when f_raise_exc_if_version_not_supported is turned on.
    """
    extractor = F360MUDPExtractor(f_raise_exc_if_version_not_supported=True)
    with pytest.raises(VersionNotSupportedException):
        extractor.extract_data(parsed_data_2)


def test_warn_if_version_not_supported(parsed_data_2):
    """
    Tests if there is only a warning when no builder supports this version of parsed data
    and f_raise_exc_if_version_not_supported is turned off.
    """
    extractor = F360MUDPExtractor(f_raise_exc_if_version_not_supported=False)
    with pytest.warns(UserWarning):
        extractor.extract_data(parsed_data_2)
