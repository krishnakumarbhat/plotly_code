# coding=utf-8
"""
F360 Execution Time Info Data Set
"""
from aspe.extractors.Interfaces.IDataSet import IDataSet


class F360ExecutionTimeInfo(IDataSet):
    """
    F360 Execution Time Info data set class
    """
    def __init__(self):
        super().__init__()

    def get_base_name(self):
        """
        TODO: consider instance.__class__.__name__? should we overload str() function to get this? CEA-243
        Get Execution Time Info base name
        :return: execution time info base name
        """
        return 'F360ExecutionTimeInfo'

    def interpolate_values(self, new_timestamp_vals, new_scan_index_vals, timestamp_signal_name='timestamp'):
        pass
