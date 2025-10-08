# coding=utf-8
"""
F360 MUDP sensors builder selector
"""
from aspe.extractors.F360.Mudp.Builders.Sensors.F360MudpSensorsBuilderV13 import F360MudpSensorsBuilderV13
from aspe.extractors.F360.Mudp.Builders.Sensors.F360MudpSensorsBuilderV14 import F360MudpSensorsBuilderV14
from aspe.extractors.F360.Mudp.Builders.Sensors.F360MudpSensorsBuilderV15 import F360MudpSensorsBuilderV15
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


class F360MudpSensorsBuilderSelector(IMudpBuilderSelector):
    """
    Sensors builder selector for F360 MUDP data.
    """
    required_stream_numbers = {72}
    legacy_stream_numbers = {3}

    available_builders = (
        F360MudpSensorsBuilderV15,
        F360MudpSensorsBuilderV14,
        F360MudpSensorsBuilderV13,
    )
