# coding=utf-8
"""
Functional tests for IMudpBuilder.
"""
from dataclasses import dataclass

import pytest

from aspe.extractors.Mudp.IMudpBuilder import IMudpBuilder


@pytest.fixture
def parsed_data():

    @dataclass
    class Header:
        configuration_file_path: str

    return {
        'header_data': {
            1: Header(r'\\127.0.0.1\stream_definitions\strdef_src035_str001_ver011.txt'),
            3: Header(r'\\127.0.0.1\stream_definitions\strdef_src035_str003_ver012.txt'),
            4: Header(r'\\127.0.0.1\stream_definitions\strdef_src035_str004_ver013.txt'),
            6: Header(r'\\127.0.0.1\stream_definitions\strdef_src035_str006_ver014.txt'),
            9: Header(r'\\127.0.0.1\stream_definitions\strdef_src035_str009_ver015.txt'),
        }
    }


class IMudpSomethingBuilder(IMudpBuilder):
    def build(self):
        pass


class Builder0(IMudpSomethingBuilder):
    required_stream_definitions = set()


class Builder1(IMudpSomethingBuilder):
    required_stream_definitions = {'strdef_src035_str001_ver011'}


class Builder2(IMudpSomethingBuilder):
    required_stream_definitions = {'strdef_src035_str001_ver012'}


class Builder3(IMudpSomethingBuilder):
    required_stream_definitions = {
        'strdef_src035_str001_ver011',
        'strdef_src035_str001_ver012'
    }


class Builder4(IMudpSomethingBuilder):
    required_stream_definitions = {
        'strdef_src035_str001_ver011',
        'strdef_src035_str003_ver012'
    }


class Builder5(IMudpSomethingBuilder):
    required_stream_definitions = {
        'strdef_src035_str001_ver011',
        'strdef_src035_str003_ver013'
    }


class Builder6(IMudpSomethingBuilder):
    required_stream_definitions = {
        'strdef_src035_str001_ver011',
        'strdef_src035_str003_ver012',
        'strdef_src035_str004_ver013',
        'strdef_src035_str006_ver014',
        'strdef_src035_str009_ver015',
    }


class Builder7(IMudpSomethingBuilder):
    required_stream_definitions = {
        'strdef_src035_str001_ver011',
        'strdef_src035_str003_ver012',
        'strdef_src035_str004_ver013',
        'strdef_src035_str006_ver014',
        'strdef_src035_str009_ver015',
        'strdef_src035_str009_ver016',
    }


class Builder8(IMudpSomethingBuilder):
    required_stream_definitions = {
        'strdef_src035_str001_ver011',
        'strdef_src035_str003_ver012',
        'strdef_src035_str004_ver013',
        'strdef_src035_str006_ver014',
        'strdef_src035_str009_ver016',
    }


class Builder9(IMudpSomethingBuilder):
    required_stream_definitions = {
        'strdef_src035_str001_ver011',
        'strdef_src035_str003_ver012',
        'strdef_src035_str004_ver013',
        'strdef_src035_str006_ver014',
        'strdef_src035_str009_ver015',
        'strdef_src035_str010_ver016',
    }


@pytest.mark.parametrize('builder_class, expected_output', [
    pytest.param(Builder0, True, id='no versions'),
    pytest.param(Builder1, True, id='one good version'),
    pytest.param(Builder2, False, id='one wrong version'),
    pytest.param(Builder3, False, id='two versions for one stream'),
    pytest.param(Builder4, True, id='two good versions'),
    pytest.param(Builder5, False, id='one good and one wrong version'),
    pytest.param(Builder6, True, id='all parsed stream versions'),
    pytest.param(Builder7, False, id='more versions than parsed'),
    pytest.param(Builder8, False, id='all parsed streams but one with wrong version'),
    pytest.param(Builder9, False, id='all parsed streams but one with two versions'),
])
def test_can_handle(builder_class, parsed_data, expected_output):
    assert builder_class.can_handle(parsed_data) == expected_output
