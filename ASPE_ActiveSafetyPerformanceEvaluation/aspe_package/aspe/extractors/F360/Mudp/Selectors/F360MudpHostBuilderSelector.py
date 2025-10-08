# coding=utf-8
"""
F360 MUDP host builder selector
"""
from aspe.extractors.F360.Mudp.Builders.Host.F360MudpHostBuilderV1 import F360MudpHostBuilderV1
from aspe.extractors.F360.Mudp.Builders.Host.F360MudpHostBuilderV3 import F360MudpHostBuilderV3
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


class F360MudpHostBuilderSelector(IMudpBuilderSelector):
    """
    Host builder selector for F360 MUDP data.
    """
    required_stream_numbers = {4, 7}
    legacy_stream_numbers = {}

    available_builders = (
        F360MudpHostBuilderV3,
        F360MudpHostBuilderV1,
    )
