from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class SDB_ExtractedData(ExtractedData):
    def __init__(self):
        super().__init__()
        self.objects_header = None
        self.metadata = None
        self.objects = None
        self.objects_header = None
        self.detections = None
        self.detections_header = None
        self.sensors = None
        self.host = None
