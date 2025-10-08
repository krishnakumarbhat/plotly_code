from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class F360Mf4BmwExtractedData(ExtractedData):
    def __init__(self):
        super().__init__()
        self.objects_header = None
        self.ptp_data = None