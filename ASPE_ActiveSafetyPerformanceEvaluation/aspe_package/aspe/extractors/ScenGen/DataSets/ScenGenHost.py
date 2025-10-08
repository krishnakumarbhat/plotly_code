"""
Scenario Generator Host Data Set
"""

from aspe.extractors.Interfaces.IHost import IHost


class ScenGenHost(IHost):
    def __init__(self):
        super().__init__()
