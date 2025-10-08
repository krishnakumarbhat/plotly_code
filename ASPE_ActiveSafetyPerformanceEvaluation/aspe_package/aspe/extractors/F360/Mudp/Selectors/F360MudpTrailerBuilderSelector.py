# coding=utf-8
"""
F360 MUDP trailer builder selector
"""
from aspe.extractors.F360.Mudp.Builders.Host.F360MudpTrailerBuilderV1 import F360MudpTrailerBuilderV1
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


class F360MudpTrailerBuilderSelector(IMudpBuilderSelector):
    """ Trailer builder selector for F360 MUDP data. """
    required_stream_numbers = {86, 4}
    legacy_stream_numbers = {}

    available_builders = (
        F360MudpTrailerBuilderV1,
    )
