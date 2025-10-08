# coding=utf-8
"""
F360 Detections Clusters Data Set
"""
from aspe.extractors.F360.DataSets.SignalDescription import F360CoreSignals
from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals


class F360DetectionsClusters(IDataSet):
    """
    F360 Detections Clusters Data Set class
    """
    def __init__(self):
        super().__init__()
        signal_names = [
            PhysicalSignals.position_x,
            PhysicalSignals.position_y,
            F360CoreSignals.tracker_id,
            PhysicalSignals.range_rate,
            PhysicalSignals.azimuth,
            GeneralSignals.existence_indicator,
            F360CoreSignals.n_dets,
        ]
        self.update_signals_definition(signal_names)

    def get_base_name(self):
        """
        Get objects base name
        :return: objects base name
        """
        return 'clusters'