# coding=utf-8
"""
F360 MUDP tracker info builder selector
"""
from aspe.extractors.F360.Mudp.Builders.TrackerInfo.F360MudpTrackerInfoBuilderV1 import F360MudpTrackerInfoBuilderV1
from aspe.extractors.F360.Mudp.Builders.TrackerInfo.F360MudpTrackerInfoBuilderV2 import F360MudpTrackerInfoBuilderV2
from aspe.extractors.F360.Mudp.Builders.TrackerInfo.F360MudpTrackerInfoBuilderV3 import F360MudpTrackerInfoBuilderV3
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


class F360MudpTrackerInfoBuilderSelector(IMudpBuilderSelector):
    """
    Tracker info builder selector for F360 MUDP data.
    """
    required_stream_numbers = {70, 7}
    legacy_stream_numbers = {5, 7}

    available_builders = (
        F360MudpTrackerInfoBuilderV3,
        F360MudpTrackerInfoBuilderV2,
        F360MudpTrackerInfoBuilderV1,
    )
