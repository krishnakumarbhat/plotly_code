"""
Scenario Generator Radar Sensors Data Set
"""

from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors


class ScenGenRadarSensors(IRadarSensors):
    def __init__(self):
        super().__init__()
