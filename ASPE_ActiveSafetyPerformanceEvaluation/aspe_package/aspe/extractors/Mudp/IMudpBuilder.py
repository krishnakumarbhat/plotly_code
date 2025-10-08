# coding=utf-8
"""
Interface for F360 MUDP builders
"""
from abc import abstractmethod

import pandas as pd

from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.Mudp.MudpSupport import get_all_stream_def_names


class IMudpBuilder(IBuilder):
    """
    Abstract builder for F360 MUDP data.
    Dataset-specific builders (e.g IF360MudpHostBuilder, IF360MudpObjectsBuilder) should inherit from this class.

    Properties:
        required_stream_definitions: collection of stream definitions required for extraction,
            e.g. ('strdef_src035_str006_ver015', 'strdef_src035_str007_ver002').
            Version-specific builders (e.g IF360MudpObjectsBuilderV15) should override this attribute.
    """
    required_stream_definitions: set = None
    legacy_stream_numbers: set = None

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = pd.DataFrame()

    @abstractmethod
    def build(self):
        """
        Extracts signals and properties from parsed data into dataset.
        Dataset-specific builders (e.g IF360MudpHostBuilder, IF360MudpObjectsBuilder) should override this method.
        :return: filled (extracted) data set
        """
        return super().build()

    @classmethod
    def can_handle(cls, parsed_data):
        """
        Checks if this version of builder is able to perform extraction on supplied parsed data.
        Calling this method does not perform building process; only prerequisites are verified.
        The default implementation compares the sets of stream definition versions that are required
        for the builder with the set of stream definition versions that were used for parsing.
        Dataset- or version-specific builders may extend or override this method to handle custom requirements logic.
        :param parsed_data: output from MUDP parser as a dict
        :return: True if parsed data can be extracted using this builder version, otherwise False
        """
        parsed_stream_definitions = get_all_stream_def_names(parsed_data)
        req_streams = cls.required_stream_definitions
        can_handle = True
        for stream in req_streams:
            if isinstance(stream, str):
                can_handle = can_handle and (stream in parsed_stream_definitions)
            elif isinstance(stream, list):
                can_handle_sub_stream = False
                for sub_stream in stream:
                    can_handle_sub_stream = can_handle_sub_stream or (sub_stream in parsed_stream_definitions)
                can_handle = can_handle and can_handle_sub_stream
        return can_handle
