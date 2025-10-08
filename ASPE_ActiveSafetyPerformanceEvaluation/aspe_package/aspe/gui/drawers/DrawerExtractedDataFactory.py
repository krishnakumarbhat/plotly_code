from aspe.evaluation.RadarObjectsEvaluation.DataStructures.PEObjectsEvaluationOutput import (
    PEObjectsEvaluationOutputSingleLog,
)
from aspe.extractors.ENV.DataSets.ENVExtractedData import ENVExtractedData
from aspe.extractors.F360.DataSets.F360Mf4BmwExtractedData import F360Mf4BmwExtractedData
from aspe.extractors.F360.DataSets.F360MudpExtractedData import F360MudpExtractedData
from aspe.extractors.F360.DataSets.F360XtrkExtractedData import F360XtrkExtractedData
from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData
from aspe.extractors.ReferenceExtractor.SDB.DataSets.SDB_ExtractedData import SDB_ExtractedData
from aspe.gui.drawers.DrawerAspeResults import DrawerAspeResults
from aspe.gui.drawers.EnvDrawer import EnvDrawer
from aspe.gui.drawers.F360BmwMidDrawer import F360BmwMidDrawer
from aspe.gui.drawers.F360MudpDrawer import F360MudpDrawer
from aspe.gui.drawers.F360XtrkDrawer import F360XtrkDrawer
from aspe.gui.drawers.NexusDrawer import NexusDrawer
from aspe.gui.drawers.PandoraDrawer import PandoraDrawer
from aspe.gui.drawers.RtRangeDrawer import RtRangeDrawer


class DrawersExtractedDataFactory:
    @staticmethod
    def create_extracted_data_drawer(drawers_controller, data_model):
        drawer = None
        if isinstance(data_model.extracted, F360MudpExtractedData):
            drawer = F360MudpDrawer("F360CoreDrawer", drawers_controller, data_model)
        elif isinstance(data_model.extracted, F360Mf4BmwExtractedData):
            drawer = F360BmwMidDrawer("F360BMWMidDrawer", drawers_controller, data_model)
        elif isinstance(data_model.extracted, SDB_ExtractedData):
            drawer = PandoraDrawer("PandoraDrawer", drawers_controller, data_model)
        elif isinstance(data_model.extracted, NexusExtractedData):
            drawer = NexusDrawer("NexusDrawer", drawers_controller, data_model)
        elif issubclass(type(data_model.extracted), F360XtrkExtractedData):
            drawer = F360XtrkDrawer("F360XtrkDrawer", drawers_controller, data_model)
        elif isinstance(data_model.extracted, ENVExtractedData):
            drawer = EnvDrawer("EnvDrawer", drawers_controller, data_model)
        elif isinstance(data_model.extracted, ExtractedData):  # RT RANGE DATA
            drawer = RtRangeDrawer("RtRangeDrawer", drawers_controller, data_model)
        elif issubclass(type(data_model.extracted), PEObjectsEvaluationOutputSingleLog):
            drawer = DrawerAspeResults("DrawerAspeResults", drawers_controller, data_model)
        return drawer
