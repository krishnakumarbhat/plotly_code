from aspe.extractors.F360.DataSets.SignalDescription import BMWMIDSignals
from aspe.extractors.Interfaces.IObjects import IObjects


class F360BmwObjectList(IObjects):
    """
    BMW someIP object list representation.
    """
    def __init__(self):
        super().__init__()
        signal_names = [BMWMIDSignals.vigem_timestamp,]
        self.update_signals_definition(signal_names)

    def get_base_name(self):
        return 'object_list'
