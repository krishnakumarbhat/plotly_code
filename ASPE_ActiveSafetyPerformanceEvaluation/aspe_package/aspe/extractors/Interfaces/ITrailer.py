# coding=utf-8
"""
Main Trailer Data Set interface
"""
from typing import Optional

from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.extractors.Interfaces.SignalDescription import PhysicalSignals


class ITrailer(IDataSet):
    """ Trailer data set interface class """
    def __init__(self):
        super(ITrailer, self).__init__()
        signal_names =[
            PhysicalSignals.position_x,
            PhysicalSignals.position_y,
            PhysicalSignals.bounding_box_dimensions_y,
            PhysicalSignals.bounding_box_dimensions_x,
            PhysicalSignals.bounding_box_orientation,

        ]
        self.update_signals_definition(signal_names)
        self.coordinate_system: Optional[str] = None
        self.bounding_box_dimensions_x: Optional[float] = None
        self.bounding_box_dimensions_y: Optional[float] = None
        self.bounding_box_orientation: Optional[float] = None
        self.bounding_box_refpoint_para_offset_ratio: Optional[float] = None
        self.bounding_box_refpoint_orh_offset_ratio: Optional[float] = None
        self.dist_of_rear_axle_to_front_bumper: Optional[float] = None

    def get_base_name(self):
        """
        Get base name of Host interface

        :return: str: Host base data set name
        """
        return 'trailer'
