from aspe.gui.drawers.abstract_drawers.DrawerExtractedData import DrawerExtractedData
from aspe.gui.drawers.DrawersFactory import DrawersFactory
from aspe.gui.drawers.F360MudpDrawer import calc_dets_azimuth_vcs


class F360BmwMidDrawer(DrawerExtractedData):
    def __init__(self, name, drawers_controller, data_model):
        calc_dets_azimuth_vcs(data_model.extracted)
        super().__init__(name, drawers_controller, data_model)

    def create_drawers(self):
        drawers = []
        if self.data_model.extracted.objects is not None:
            drawers.append(DrawersFactory.create_objects_drawer(self, self.data_model.extracted.objects, "ObjectList"))
        if self.data_model.extracted.detections is not None:
            drawers.append(DrawersFactory.create_detection_drawer(self,
                                                                  self.data_model.extracted.detections,
                                                                  "Detections",
                                                                  "range_rate",
                                                                  "azimuth_vcs"))
        return drawers
