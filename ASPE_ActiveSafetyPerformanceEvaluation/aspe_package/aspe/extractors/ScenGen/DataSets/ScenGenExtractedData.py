from aspe.extractors.Interfaces.ExtractedData import ExtractedData


class ScenGenExtractedData(ExtractedData):
    def __init__(self):
        super(ScenGenExtractedData, self).__init__()

        self.contra_reference = None
