# coding=utf-8
"""
Functional tests for IMudpBuilderSelector.
"""
from dataclasses import dataclass

import pytest

from aspe.extractors.F360.Mudp.Exceptions.MissingStreamException import MissingStreamException
from aspe.extractors.F360.Mudp.Exceptions.VersionNotSupportedException import VersionNotSupportedException
from aspe.extractors.Mudp.IMudpBuilder import IMudpBuilder
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


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


@pytest.mark.parametrize('_required_stream_numbers, _expected_missing_stream_numbers', [
    pytest.param({1, 3, 4, 6, 9, 10}, {10}),
    pytest.param({10}, {10}),
    pytest.param({10, 11}, {10, 11}),
    pytest.param({9, 10}, {10}),
])
def test_raise_missing_stream_exception(_required_stream_numbers, _expected_missing_stream_numbers, parsed_data):
    class MudpSomethingBuilderSelector(IMudpBuilderSelector):
        required_stream_numbers = _required_stream_numbers

    selector = MudpSomethingBuilderSelector()
    with pytest.raises(MissingStreamException) as exc_info:
        selector.select_builder(parsed_data)

    exception = exc_info.value
    assert exception.missing_stream_numbers == _expected_missing_stream_numbers


class IMudpSomethingBuilder(IMudpBuilder):
    def build(self):
        pass


class Selector1(IMudpBuilderSelector):
    available_builders = []


class Selector2(IMudpBuilderSelector):
    class BuilderV99(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str004_ver099'}

    available_builders = [BuilderV99]


class Selector3(IMudpBuilderSelector):
    class BuilderV98(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str004_ver099'}

    class BuilderV97(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str004_ver098'}

    class BuilderV96(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str004_ver097'}

    available_builders = [BuilderV98, BuilderV97, BuilderV96]


class Selector4(IMudpBuilderSelector):
    class BuilderV95(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str004_ver013', 'strdef_src035_str006_ver096'}

    available_builders = [BuilderV95]


@pytest.mark.parametrize('selector_class', [
    pytest.param(Selector1, id='no builders'),
    pytest.param(Selector2, id='wrong version'),
    pytest.param(Selector3, id='wrong versions'),
    pytest.param(Selector4, id='other stream wrong version'),
])
def test_raise_version_not_supported_exception(selector_class, parsed_data):
    selector = selector_class()
    with pytest.raises(VersionNotSupportedException):
        selector.select_builder(parsed_data)


class Selector11(IMudpBuilderSelector):
    class BuilderV89(IMudpSomethingBuilder):
        required_stream_definitions = set()

    available_builders = [BuilderV89]


class Selector12(IMudpBuilderSelector):
    class BuilderV88(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str001_ver011'}

    available_builders = [BuilderV88]


class Selector13(IMudpBuilderSelector):
    class BuilderV87(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str001_ver012'}

    class BuilderV86(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str001_ver011'}

    available_builders = [BuilderV87, BuilderV86]


class Selector14(IMudpBuilderSelector):
    class BuilderV85(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str001_ver011'}

    class BuilderV84(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str001_ver011'}

    available_builders = [BuilderV85, BuilderV84]


class Selector15(IMudpBuilderSelector):
    class BuilderV83(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str001_ver011', 'strdef_src035_str003_ver012', 'strdef_src035_str004_ver013'}

    class BuilderV82(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str001_ver011'}

    available_builders = [BuilderV83, BuilderV82]


class Selector16(IMudpBuilderSelector):
    class BuilderV81(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str001_ver011', 'strdef_src035_str003_ver013'}

    class BuilderV80(IMudpSomethingBuilder):
        required_stream_definitions = {'strdef_src035_str001_ver011'}

    available_builders = [BuilderV81, BuilderV80]


@pytest.mark.parametrize('selector_class, expected_builder_class', [
    pytest.param(Selector11, Selector11.BuilderV89, id='match builder with no requirements'),
    pytest.param(Selector12, Selector12.BuilderV88, id='match builder that meets the requirements'),
    pytest.param(Selector13, Selector13.BuilderV86, id='compare stream definition versions'),
    pytest.param(Selector14, Selector14.BuilderV85, id='match first found'),
    pytest.param(Selector15, Selector15.BuilderV83, id='match first found even if has more requirements'),
    pytest.param(Selector16, Selector16.BuilderV80, id='match only if all requirements are satisfied'),
])
def test_appropriate_builder_selected(selector_class, parsed_data, expected_builder_class):
    selector = selector_class()
    assert selector.select_builder(parsed_data) == expected_builder_class
