from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors


class ENVRadarSensors(IRadarSensors):
    """Represents OccupancyGrid radar sensors"""
    def __init__(self):
        super().__init__()
