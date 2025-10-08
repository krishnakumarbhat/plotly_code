# coding=utf-8
"""
F360 MUDP internal objects builder selector
"""
from aspe.extractors.F360.Mudp.Builders.InternalObjects.F360MudpInternalObjectsBuilderV12 import (
    F360MudpInternalObjectsBuilderV12,
)
from aspe.extractors.F360.Mudp.Builders.InternalObjects.F360MudpInternalObjectsBuilderV14 import (
    F360MudpInternalObjectsBuilderV14,
)
from aspe.extractors.F360.Mudp.Builders.InternalObjects.F360MudpInternalObjectsBuilderV15 import (
    F360MudpInternalObjectsBuilderV15,
)
from aspe.extractors.F360.Mudp.Builders.InternalObjects.F360MudpInternalObjectsBuilderV16 import (
    F360MudpInternalObjectsBuilderV16,
)
from aspe.extractors.Mudp.IMudpBuilderSelector import IMudpBuilderSelector


class F360MudpInternalObjectsBuilderSelector(IMudpBuilderSelector):
    """
    Internal objects builder selector for F360 MUDP data.
    """
    required_stream_numbers = {70}
    legacy_stream_numbers = {4, 5, 7}

    available_builders = (
        F360MudpInternalObjectsBuilderV16,
        F360MudpInternalObjectsBuilderV15,
        F360MudpInternalObjectsBuilderV14,
        F360MudpInternalObjectsBuilderV12,
    )
