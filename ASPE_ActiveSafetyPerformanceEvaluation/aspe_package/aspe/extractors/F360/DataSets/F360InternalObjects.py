# coding=utf-8
"""
F360 Internal Objects Data Set
"""
from aspe.extractors.F360.DataSets.SignalDescription import F360CoreSignals
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Interfaces.SignalDescription import PhysicalSignals


class F360InternalObjects(IObjects):
    """
    Class which represents f360 internal objects data. It extends basic objects data structure.
    """
    def __init__(self):
        super().__init__()
        signal_names = [
            F360CoreSignals.filter_type,
            F360CoreSignals.reduced_id,
            F360CoreSignals.status,
            PhysicalSignals.curvature,
            F360CoreSignals.confidence_level,
            F360CoreSignals.n_dets,
            F360CoreSignals.f_moving,
            F360CoreSignals.f_moveable,
        ]
        self.update_signals_definition(signal_names)

    def get_base_name(self):
        """
        Get base name of internal objects
        :return: str
        """
        return 'internal_objects'
