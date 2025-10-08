from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class F360MudpExtractedData(ExtractedData):
    def __init__(self):
        super().__init__()
        self.tracker_info = None
        self.execution_time_info = None
        self.oal_objects = None
        self.oal_objects_header = None
        self.gdsr_output = None
        self.static_environment = None
