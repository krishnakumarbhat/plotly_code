# coding=utf-8
"""
F360 Tracker Info Data Set
"""
import pandas as pd

from aspe.extractors.F360.DataSets.SignalDescription import BMWMIDSignals
from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription


class PTPData(IDataSet):
    """
    F360 Tracker Info data set class
    """
    def __init__(self):
        super().__init__()
        signal_names = [
            BMWMIDSignals.vigem_timestamp,
            BMWMIDSignals.ethernet_timestamp,
            BMWMIDSignals.vigem_to_global_time_diff,
        ]
        self.update_signals_definition(signal_names)

        self.vigem_to_global_time_diff_median = None
        self.vigem_to_global_time_diff_spread = None

    def interpolate_values(self, new_timestamp_vals, new_scan_index_vals, timestamp_signal_name='timestamp'):
        pass

    def get_base_name(self):
        """
        TODO: consider instance.__class__.__name__? should we overload str() function to get this? CEA-243
        Get Tracker Info base name
        :return: tracker info base name
        """
        return 'PTPData'
