# coding=utf-8
"""
F360 Reduced Objects Data Set
"""
from aspe.extractors.F360.DataSets.SignalDescription import F360CoreSignals
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals


class F360Objects(IObjects):
    """
    F360 Reduced Objects Data Set class
    """
    def __init__(self):
        super().__init__()
        signal_names = [
            F360CoreSignals.filter_type,
            GeneralSignals.id,
            F360CoreSignals.status,
            PhysicalSignals.curvature,
            F360CoreSignals.confidence_level,
            F360CoreSignals.n_dets,
            F360CoreSignals.f_moving,
            F360CoreSignals.f_moveable,
            F360CoreSignals.f360_object_class,
        ]
        self.update_signals_definition(signal_names)

