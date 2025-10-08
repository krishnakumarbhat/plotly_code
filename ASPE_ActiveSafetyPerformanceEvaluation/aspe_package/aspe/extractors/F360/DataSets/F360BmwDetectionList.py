from aspe.extractors.F360.DataSets.SignalDescription import BMWMIDSignals
from aspe.extractors.Interfaces.IRadarDetections import IRadarDetections


class F360BmwDetectionList(IRadarDetections):
    """
    BMW someIP detection list representation.
    """
    def __init__(self):
        super().__init__()
        signal_names = [BMWMIDSignals.vigem_timestamp]
        self.update_signals_definition(signal_names)

    def get_base_name(self):
        return 'detection_list'
