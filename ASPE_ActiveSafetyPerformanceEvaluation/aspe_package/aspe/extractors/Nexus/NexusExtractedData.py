from aspe.extractors import ExtractedData


class NexusExtractedData(ExtractedData):
    def __init__(self):
        super().__init__()
        self.auto_gt_lidar_cuboids = None