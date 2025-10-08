# coding=utf-8
"""
F360 MUDP execution time info builder selector
"""
from aspe.extractors.F360.Mudp.Builders.ExecutionTimeInfo.F360MudpExecutionTimeInfoBuilderV11 import (
    F360MudpExecutionTimeInfoBuilderV11,
)
from aspe.extractors.F360.Mudp.Builders.ExecutionTimeInfo.F360MudpExecutionTimeInfoBuilderV12 import (
    F360MudpExecutionTimeInfoBuilderV12,
)
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


class F360MudpExecutionTimeInfoBuilderSelector(IMudpBuilderSelector):
    """
    Execution time info builder selector for F360 MUDP data.
    """
    required_stream_numbers = {9}
    legacy_stream_numbers = {}

    available_builders = (
        F360MudpExecutionTimeInfoBuilderV12,
        F360MudpExecutionTimeInfoBuilderV11,
    )
