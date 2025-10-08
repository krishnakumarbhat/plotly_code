from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors


class GdsrRadarSensors(IRadarSensors):
    """Represents GDSR radar sensors"""
    def __init__(self):
        super().__init__()
