from aspe.gui.drawers.abstract_drawers.DrawerExtractedData import DrawerExtractedData
from aspe.gui.drawers.DrawersFactory import DrawersFactory


class RtRangeDrawer(DrawerExtractedData):
    def __init__(self, name, drawers_controller, data_model):
        super().__init__(name, drawers_controller, data_model)

    def create_drawers(self):
        drawers = []
        if self.data_model.extracted.host is not None:
            drawers.append(DrawersFactory.create_host_drawer(self, self.data_model.extracted.host, "Hunter"))
        if self.data_model.extracted.objects is not None:
            drawers.append(DrawersFactory.create_objects_drawer(self, self.data_model.extracted.objects, "Target"))
        return drawers
