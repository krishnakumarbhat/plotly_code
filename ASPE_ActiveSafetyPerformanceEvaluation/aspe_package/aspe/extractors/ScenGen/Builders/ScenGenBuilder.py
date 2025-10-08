"""
General builder for Scenario Generator data.
"""

from aspe.extractors.Interfaces.IBuilder import IBuilder


class ScenGenBuilder(IBuilder):
    def __init__(self, parsed_data, **kwargs):
        super(ScenGenBuilder, self).__init__(parsed_data, **kwargs)