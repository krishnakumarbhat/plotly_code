# coding=utf-8
"""
F360 Xtrk (debug) tracked objects Data Set
"""
from aspe.extractors.F360.DataSets.F360Objects import F360Objects
from aspe.extractors.F360.DataSets.SignalDescription import F360XtrkSignals


class F360XtrkObjects(F360Objects):
    """
    F360 Xtrk (debug) tracked objects Data Set class
    """
    def __init__(self):
        super().__init__()
        signal_names = [
            F360XtrkSignals.assoc_gates_center_x,
            F360XtrkSignals.assoc_gates_center_y,
            F360XtrkSignals.assoc_gates_length,
            F360XtrkSignals.assoc_gates_width,
            F360XtrkSignals.pseudo_position_x,
            F360XtrkSignals.pseudo_position_y,
        ]
        self.update_signals_definition(signal_names)
