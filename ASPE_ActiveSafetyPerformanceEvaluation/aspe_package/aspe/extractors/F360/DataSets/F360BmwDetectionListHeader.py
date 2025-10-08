from aspe.extractors.F360.DataSets.SignalDescription import BMWMIDSignals
from aspe.extractors.Interfaces.IDataSet import IDataSet


class F360BmwDetectionListHeader(IDataSet):
    def __init__(self):
        super().__init__()
        self.raw_signals = None
        signal_names = [
            BMWMIDSignals.vigem_timestamp,
            BMWMIDSignals.publish_time_in_global_domain,
            BMWMIDSignals.system_latency,
        ]
        self.update_signals_definition(signal_names)

    def get_base_name(self):
        return 'detection_list_header'
