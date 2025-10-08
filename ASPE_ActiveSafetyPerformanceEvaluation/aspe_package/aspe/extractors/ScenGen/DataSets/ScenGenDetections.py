"""
Scenario Generator Detections Data Set
"""

from aspe.extractors.Interfaces.IRadarDetections import IRadarDetections
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals


class ScenGenDetections(IRadarDetections):
    def __init__(self):
        super().__init__()

        signal_names = [GeneralSignals.look_id]
        self.update_signals_definition(signal_names)