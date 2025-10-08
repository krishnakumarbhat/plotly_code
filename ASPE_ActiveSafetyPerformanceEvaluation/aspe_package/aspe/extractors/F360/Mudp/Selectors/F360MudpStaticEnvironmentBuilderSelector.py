# coding=utf-8
"""
F360 MUDP static environment builder selector
"""
from aspe.extractors.F360.Mudp.Builders.static_env.F360MudpStaticEnvironmentBuilderV2 import (
    F360MudpStaticEnvironmentBuilderV2,
)
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


class F360MudpStaticEnvironmentBuilderSelector(IMudpBuilderSelector):
    """
    Static environment builder selector for F360 MUDP data.
    """
    required_stream_numbers = {19}
    legacy_stream_numbers = {}

    available_builders = (
        F360MudpStaticEnvironmentBuilderV2,
    )
