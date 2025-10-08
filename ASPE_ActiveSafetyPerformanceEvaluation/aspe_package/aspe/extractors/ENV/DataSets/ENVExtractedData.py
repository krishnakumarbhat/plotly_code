from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class ENVExtractedData(ExtractedData):
    def __init__(self):
        super().__init__()
        self.stationary_geometries = None
        self.occupancy_grid = None