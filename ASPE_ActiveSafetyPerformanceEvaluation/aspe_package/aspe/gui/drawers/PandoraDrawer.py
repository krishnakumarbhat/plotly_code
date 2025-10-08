from aspe.gui.drawers.abstract_drawers.DrawerExtractedData import DrawerExtractedData
from aspe.gui.drawers.DrawersFactory import DrawersFactory


class PandoraDrawer(DrawerExtractedData):
    def __init__(self, name, drawers_controller, data_model):
        super().__init__(name, drawers_controller, data_model)

    def create_drawers(self):
        drawers = []
        if self.data_model.extracted.objects is not None:
            drawers.append(DrawersFactory.create_objects_drawer(self, self.data_model.extracted.objects, "Cuboids"))
        return drawers
