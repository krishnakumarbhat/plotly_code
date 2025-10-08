# coding=utf-8
"""
F360 MUDP detections builder selector
"""
from aspe.extractors.F360.Mudp.Builders.Detections.F360MudpDetectionsBuilderV13 import F360MudpDetectionsBuilderV13
from aspe.extractors.F360.Mudp.Builders.Detections.F360MudpDetectionsBuilderV14 import F360MudpDetectionsBuilderV14
from aspe.extractors.F360.Mudp.Builders.Detections.F360MudpDetectionsBuilderV15 import F360MudpDetectionsBuilderV15
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


class F360MudpDetectionsBuilderSelector(IMudpBuilderSelector):
    """
    Detections builder selector for F360 MUDP data.
    """
    required_stream_numbers = {71, 72}
    legacy_stream_numbers = {3, 7}

    available_builders = (
        F360MudpDetectionsBuilderV15,
        F360MudpDetectionsBuilderV14,
        F360MudpDetectionsBuilderV13,
    )
