# coding=utf-8
"""
F360 Tracker Info Data Set
"""
from typing import Optional

from aspe.extractors.Interfaces.IDataSet import IDataSet


class F360TrackerInfo(IDataSet):
    """
    F360 Tracker Info data set class
    """
    def __init__(self):
        super().__init__()
        self.sw_version: Optional[str] = None
        self.build_id: Optional[str] = None

    def get_base_name(self):
        """
        TODO: consider instance.__class__.__name__? should we overload str() function to get this? CEA-243
        Get Tracker Info base name
        :return: tracker info base name
        """
        return 'F360TrackerInfo'
