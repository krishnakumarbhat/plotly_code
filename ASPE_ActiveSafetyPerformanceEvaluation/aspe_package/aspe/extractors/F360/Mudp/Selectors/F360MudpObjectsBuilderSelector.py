# coding=utf-8
"""
F360 MUDP objects builder selector
"""
from aspe.extractors.F360.Mudp.Builders.Objects.F360MudpObjectsBuilderV13 import F360MudpObjectsBuilderV13
from aspe.extractors.F360.Mudp.Builders.Objects.F360MudpObjectsBuilderV15 import F360MudpObjectsBuilderV15
from aspe.extractors.F360.Mudp.Builders.Objects.F360MudpObjectsBuilderV16 import F360MudpObjectsBuilderV16
from aspe.extractors.F360.Mudp.Builders.Objects.F360MudpObjectsBuilderV17 import F360MudpObjectsBuilderV17
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


class F360MudpObjectsBuilderSelector(IMudpBuilderSelector):
    """
    Objects builder selector for F360 MUDP data.
    """
    required_stream_numbers = {70}
    legacy_stream_numbers = {4, 6, 7}

    available_builders = (
        F360MudpObjectsBuilderV17,
        F360MudpObjectsBuilderV16,
        F360MudpObjectsBuilderV15,
        F360MudpObjectsBuilderV13,
    )
