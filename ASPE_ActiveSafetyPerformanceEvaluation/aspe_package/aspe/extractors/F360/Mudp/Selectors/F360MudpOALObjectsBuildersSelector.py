# coding=utf-8
"""
F360 MUDP objects builder selector
"""
from aspe.extractors.F360.Mudp.Builders.OAL.F360MudpOALObjectsBuilderStream17V5 import (
    F360MudpOALObjectsBuilderStream17V5,
)
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


class F360MudpOALObjectsBuilderSelector(IMudpBuilderSelector):
    """
    Output Adaptation Layer objects builder selector for F360 MUDP data.
    """
    required_stream_numbers = {17}
    legacy_stream_numbers = {}

    available_builders = (
        F360MudpOALObjectsBuilderStream17V5,
    )
