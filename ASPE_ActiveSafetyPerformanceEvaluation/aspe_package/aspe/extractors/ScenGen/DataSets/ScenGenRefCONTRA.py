"""
Scenario Generator CONTRA reference Data Set
"""

from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.extractors.Interfaces.SignalDescription import PhysicalSignals
from aspe.extractors.ScenGen.DataSets.SignalDescription import ScenGenRefCONTRASignals


class ScenGenRefCONTRA(IDataSet):
    def __init__(self):
        super().__init__()
        signal_names = [
            PhysicalSignals.position_x,
            PhysicalSignals.position_y,
            ScenGenRefCONTRASignals.vertex_id,
            ScenGenRefCONTRASignals.contour_id,
        ]

        self.update_signals_definition(signal_names)

    def get_base_name(self):
        return 'RefCONTRA'