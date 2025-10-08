from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class F360XtrkExtractedData(ExtractedData):
    def __init__(self):
        super().__init__()
        self.tracker_info = None
        self.clusters = None
        self.historical_detections = None
        self.static_environment = None
